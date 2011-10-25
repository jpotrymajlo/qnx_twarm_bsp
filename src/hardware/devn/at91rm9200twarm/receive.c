/*
 * $QNXLicenseC:
 * Copyright 2008 QNX Software Systems Gmbh & Co. KG. All rights reserved.
 * This software may not be used in, licensed for use with or otherwise
 * distributed for use with oxymeter products. It is otherwise licensed
 * under the Apache License, Version 2.0 (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless
 * required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied. See the License for
 * the specific language governing permissions and limitations under the
 * License.
 * $
 */


#include "at91sam.h"

static npkt_t *
at91sam_assemble(at91sam_dev_t *at91sam)
{
	npkt_t		*rpkt = NULL;
	char		*dst  = NULL;
	char		*src  = NULL;
    size_t      len;
	net_buf_t	*buf;
    uint32_t    size;
    uint32_t    recv_len = 0;
    uint32_t    phy_addr;

    /* take a packet off the free queue */
    pthread_mutex_lock(&at91sam->rx_pkt_q_mutex);

    if (rpkt = at91sam->rx_free_pkt_q) {
        at91sam->rx_free_pkt_q = rpkt->next;
        rpkt->next = NULL;
        at91sam->num_rx_free--;
        pthread_mutex_unlock(&at91sam->rx_pkt_q_mutex);
    } else {
        pthread_mutex_unlock(&at91sam->rx_pkt_q_mutex);

        if ((rpkt = at91sam_alloc_npkt(at91sam)) == NULL) {
            at91sam->stats.rx_failed_allocs++;
            goto frag_fail;
        }
    }

    /* Assemble all buffer received for this packet and put them in the
       same packet */
    buf = TAILQ_FIRST(&rpkt->buffers);
    buf->net_iov->iov_len = 0;
	dst = buf->net_iov->iov_base;

    /* Get information about buffer */
    size = at91sam->rbds->size;
    len  = AT91RM9200_RX_BUF_SIZE;

    recv_len = (size & AT91RM9200_RBD_LEN_MASK)  - 4;
    len      =  recv_len % AT91RM9200_RX_BUF_SIZE;

    /* Get virtual address for current buffer */
    phy_addr = (uint32_t)at91sam->rbds->addr & ~(AT91RM9200_RBD_DONE | AT91RM9200_RBD_WRAP);
    src = (char*)(at91sam->rbds_buf_vbase + (phy_addr - at91sam->rbds_buf_pbase));

    /* Copy data into buffer */
    memcpy(dst, src, len);
    dst      += len;

        /* Release BD */
    at91sam->rbds->size  = 0;
    at91sam->rbds->addr &= ~AT91RM9200_RBD_DONE;


    /* Update packet info */
    rpkt->framelen = buf->net_iov->iov_len = recv_len;
    rpkt->iface = 0;
    rpkt->next = NULL;

    if (at91sam->n_prom)
        rpkt->flags |= _NPKT_PROMISC;

    /* Process frame status from last received buffer */
    at91sam->stats.octets_rxed_ok += rpkt->framelen;

    if (size & AT91RM9200_RBD_BCAST)
        at91sam->stats.rxed_broadcast++;
    else if (size & AT91RM9200_RBD_MCAST)
        at91sam->stats.rxed_multicast++;


	return rpkt;

frag_fail:

    if (rpkt != NULL) {
        at91sam_receive_complete(rpkt, at91sam, at91sam);
    }
    return NULL;
}



void
at91sam_receive(at91sam_dev_t *at91sam)
{
    npkt_t		    *rpkt = NULL;
    volatile BD_t   *pBD;

    pBD = at91sam->rbds;
    while (pBD->addr & AT91RM9200_RBD_DONE) {

        if ((rpkt = at91sam_assemble(at91sam))) {

            /* Send it up */
            if (rpkt = at91sam->ion->tx_up_start(at91sam->reg_hdl,
                rpkt, 0, 0, at91sam->cell, at91sam->cfg.lan, 0, at91sam)) {
                nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "Fail to send Rx PKT up");
                at91sam_receive_complete(rpkt, at91sam, at91sam);
            }
        }

    	pBD->addr &= ~AT91RM9200_RBD_DONE;
        /* Move to next BD */
        if (pBD->addr & AT91RM9200_RBD_WRAP) {
            pBD = at91sam->rbds_base;
        } else {
            pBD++;
        }
        at91sam->rbds = pBD;
    }

}

int
at91sam_receive_complete(npkt_t *npkt, void *done_hdl, void *func_hdl)
{
	at91sam_dev_t	*at91sam = done_hdl;
	pthread_mutex_lock(&at91sam->rx_pkt_q_mutex);
	if (at91sam->num_rx_free < at91sam->num_rx_descriptors) {
		net_buf_t	*buf = TAILQ_FIRST(&npkt->buffers);
		net_iov_t	*iov = buf->net_iov;

		npkt->ref_cnt = 1;
		npkt->req_complete = 0;
		npkt->flags = _NPKT_UP;
		npkt->tot_iov = 1;

		buf->niov     = 1;

		iov->iov_base = (void *)npkt->org_data;
		iov->iov_phys = (paddr_t)at91sam->ion->mphys(iov->iov_base);
		iov->iov_len  = AT91RM9200_MAX_FRAME_LEN;

		npkt->next = at91sam->rx_free_pkt_q;
		at91sam->rx_free_pkt_q = npkt;
		at91sam->num_rx_free++;
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "Receive complete");

		pthread_mutex_unlock(&at91sam->rx_pkt_q_mutex);
	} else {
		pthread_mutex_unlock(&at91sam->rx_pkt_q_mutex);
		at91sam->ion->free(npkt->org_data);
		at91sam->ion->free(npkt);
	}
	return 0;
}
