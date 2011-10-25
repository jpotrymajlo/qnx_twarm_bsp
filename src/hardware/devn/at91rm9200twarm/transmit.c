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


static npkt_t  *at91sam_defrag (at91sam_dev_t *at91sam, npkt_t *npkt)
{
    npkt_t			*dpkt;
    uint8_t			*dptr;
    char			*dst;
    net_iov_t		*iov;
    net_buf_t		*dov = NULL, *buf;
    int				totalLen;
    int				i, len;

	/* Setup our default return value */
	dpkt = NULL;

	/* Do some checks */
	buf = TAILQ_FIRST(&npkt->buffers);
	totalLen = 0;
	while (buf != NULL) {
		for (i = 0, iov = buf->net_iov; i < buf->niov; i++, iov++) {
			totalLen += iov->iov_len;
		}
		buf = TAILQ_NEXT(buf, ptrs);
	}


	if(totalLen <= AT91RM9200_MAX_FRAME_LEN) {
		if((dpkt = at91sam_alloc_npkt(at91sam)) != NULL) {
			dpkt->framelen  = npkt->framelen;
			dpkt->csum_flags = npkt->csum_flags;
			dpkt->flags |= AT91RM9200_DEFRAG_PACKET;
			dpkt->next = NULL;

			dov = TAILQ_FIRST(&dpkt->buffers);
			dst = dov->net_iov->iov_base;

			len = 0;
			buf = TAILQ_FIRST (&npkt->buffers);
			while (buf != NULL) {
				for (i = 0, iov = buf->net_iov; i < buf->niov; i++, iov++) {
					memcpy (dst, iov->iov_base, iov->iov_len);
					dst += iov->iov_len;
					len += iov->iov_len;
				}

				buf = TAILQ_NEXT(buf, ptrs);
			}

			dov->net_iov->iov_len = len;
		}
	}

	if (dov != NULL) {
		dptr = dov->net_iov->iov_base;
		if (dptr[0] & 1) {
			if (AT91RM9200_IS_BROADCAST(dptr))
				at91sam->stats.txed_broadcast++;
			else
				at91sam->stats.txed_multicast++;
			}
		}
	npkt->next = NULL;
	pthread_mutex_unlock(&at91sam->tx_mutex);
	at91sam->ion->tx_done(at91sam->reg_hdl, npkt);	/* tell io-net we are done */
													/* with those small packet buffers */
	pthread_mutex_lock(&at91sam->tx_mutex);
	return (dpkt);
}

static int at91sam_send(at91sam_dev_t * at91sam) {
	npkt_t		*npkt;
	uintptr_t	iobase = at91sam->iobase;
	net_buf_t   *buf;

	if (at91sam->current_frame == NULL)
	{
        /* Vefify if next BD belong to software */
        if (npkt = at91sam->nhead) {
            at91sam->nhead = npkt->next;
            npkt->next = NULL;
            if (npkt == at91sam->ntail)
                at91sam->ntail = NULL;

	    /* Unfortunately the buffers from io-net are not aligned, so we have to defrag */
	        if ((npkt = at91sam_defrag(at91sam, npkt)) == NULL) {
			    npkt->flags = _NPKT_NO_RES;
			    pthread_mutex_unlock (&at91sam->tx_mutex);
			    at91sam->ion->tx_done(at91sam->reg_hdl, npkt);
			    pthread_mutex_lock (&at91sam->tx_mutex);
	            nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-at91sam: Fail to defrag pkt.");
	            return -1;
		    }

            at91sam->stats.octets_txed_ok += npkt->framelen;
            at91sam->current_frame = npkt;

		    buf = TAILQ_FIRST(&npkt->buffers);
		    out32(iobase + AT91RM9200_TAR, (uint32_t)(uint8_t*) buf->net_iov->iov_phys);
	 	    out32(iobase + AT91RM9200_TCR, npkt->framelen);
        }
	}

	return (0);
}
/*
 * at91sam_send_packets()
 *
 * Description: entry point for Tx from the upper level
 */
int at91sam_send_packets(npkt_t *npkt, void *rx_down_hdl)
{
	at91sam_dev_t	*at91sam = rx_down_hdl;

	if (npkt->flags & _NPKT_MSG) {
		errno = EINVAL;
        if (at91sam->cfg.verbose) {
			nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR,
			  "devn-at91sam: at91sam_send_packets(): _NPKT_MSG flag set, tx failed");
		}

		return TX_DOWN_FAILED;
	} else {
		/* Data message */
		pthread_mutex_lock(&at91sam->tx_mutex);

		if (at91sam->ntail) {
			at91sam->ntail->next = npkt;
		} else {
			at91sam->nhead = npkt;
		}


		at91sam->ntail = npkt;

		at91sam_send(at91sam);

		pthread_mutex_unlock(&at91sam->tx_mutex);
	}

	return TX_DOWN_OK;
}


/* Reclaim completed packets */
int at91sam_transmit_complete(at91sam_dev_t *at91sam)
{
    uint32_t	tsr;
	uintptr_t	base = at91sam->iobase;

	tsr = in32(base + AT91RM9200_TSR);

	/* Clear transmit status */
	out32(base + AT91RM9200_TSR, tsr);

	pthread_mutex_lock(&at91sam->tx_mutex);

	at91sam->ion->free(at91sam->current_frame->org_data);
	at91sam->ion->free(at91sam->current_frame);

    /* Verify we don't have anything else to send */
	at91sam->current_frame = NULL;
	at91sam_send(at91sam);
	pthread_mutex_unlock(&at91sam->tx_mutex);

	return 0;
}




