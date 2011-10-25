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

static void
at91sam_mc_add(at91sam_dev_t *at91sam, uint8_t *mcaddr)
{
	uint8_t	bitpos = nic_calc_crc_be(mcaddr, 6) >> 26;

	at91sam->hash[bitpos >> 5] |= 1 << (bitpos & 31);
	at91sam->hcnt[bitpos]++;

	at91sam_rx_set_mode(at91sam);
}

static void
at91sam_mc_remove(at91sam_dev_t *at91sam, uint8_t *mcaddr)
{
	uint8_t	bitpos = nic_calc_crc_be(mcaddr, 6) >> 26;

	if (at91sam->hcnt[bitpos]) {
		if (--at91sam->hcnt[bitpos] == 0)
			at91sam->hash[bitpos >> 5] &= ~(1 << (bitpos & 31));
	}

	at91sam_rx_set_mode(at91sam);
}

int
handle_multicast(at91sam_dev_t *at91sam, struct _io_net_msg_mcast *msg)
{
	int	first = 1;
	int	rc = EOK;

	while (msg) {
		if (at91sam->cfg.verbose > 3)
			nic_slogf (_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: multicast msg %p - type %x - flags %x", msg, msg->type, msg->flags);

		switch (msg->type) {
		/* New multicast address to filter on */
		case _IO_NET_JOIN_MCAST:
			if (!(msg->flags & _IO_NET_MCAST_ALL)) {
				if (nic_ether_mcast_valid(msg) == -1)
					return EINVAL;
				at91sam_mc_add(at91sam, (uint8_t *)LLADDR(&msg->mc_min.addr_dl));
			}
			else if (at91sam->n_allmulti++ == 0)
				at91sam_rx_set_mode(at91sam);
			break;

		/* Removing a multicast address from the list */
		case _IO_NET_REMOVE_MCAST:
			if (!(msg->flags & _IO_NET_MCAST_ALL)) {
				if (nic_ether_mcast_valid(msg) == -1)
					return EINVAL;
				at91sam_mc_remove(at91sam, (uint8_t *)LLADDR(&msg->mc_min.addr_dl));
			} else if (at91sam->n_allmulti) {
				if (--at91sam->n_allmulti == 0)
					at91sam_rx_set_mode(at91sam);
			}

			break;

		default:
			if (at91sam->cfg.verbose)
				nic_slogf (_SLOGC_NETWORK, _SLOG_ERROR, "devn-at91sam: Message type not supported.");
			rc = ENOTSUP;
			break;
		}

		/*
		 * If the first's type is either _IO_NET_JOIN_MCAST or _IO_NET_REMOVE_MCAST
		 * we should only operate on the changing entry.  Otherwise, it is due to
		 * us asking io-net for the current database that we need to re-stuff
		 * the whole thing as it was cleared out previously.
		 */
		if (first & msg->type != 0)
			break;
		first = 0;
		msg = msg->next;
	}

	return rc;
}

int at91sam_update_stats(at91sam_dev_t *at91sam)
{
	nic_stats_t				*stats = &at91sam->stats;
	nic_ethernet_stats_t	*estats = &stats->un.estats;
	uintptr_t				base = at91sam->iobase;

	stats->txed_ok             += in32(base + AT91RM9200_FRA);
	stats->rxed_ok             += in32(base + AT91RM9200_OK);

	estats->internal_tx_errors += in32(base + AT91RM9200_TUE);
	estats->internal_rx_errors += in32(base + AT91RM9200_ROV);
	estats->no_carrier         += in32(base + AT91RM9200_CSE);
	estats->tx_deferred        += in32(base + AT91RM9200_DTE);
	estats->xcoll_aborted      += in32(base + AT91RM9200_ECOL);
	estats->late_collisions    += in32(base + AT91RM9200_LCOL);
	estats->single_collisions  += in32(base + AT91RM9200_SCOL);
	estats->multi_collisions   += in32(base + AT91RM9200_MCOL);
	estats->align_errors       += in32(base + AT91RM9200_ALE);
	estats->fcs_errors         += in32(base + AT91RM9200_SEQE);
	estats->short_packets      += in32(base + AT91RM9200_USF);

	estats->total_collision_frames = estats->late_collisions + estats->single_collisions + estats->multi_collisions;

	return EOK;
}

int
at91sam_devctl(void *hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret)
{
	at91sam_dev_t	*at91sam = (void *)hdl;
	int				status = EOK;

	switch (dcmd) {
	case DCMD_IO_NET_TX_FLUSH:
		pthread_mutex_lock(&at91sam->tx_mutex);
		at91sam_transmit_complete(at91sam);
		pthread_mutex_unlock(&at91sam->tx_mutex);
		break;

	case DCMD_IO_NET_GET_STATS:
		at91sam_update_stats(at91sam);
		memcpy(data, &at91sam->stats, sizeof(at91sam->stats));
		break;

	case DCMD_IO_NET_GET_CONFIG:
		memcpy(data, &at91sam->cfg, sizeof (at91sam->cfg));
		status = EOK;
		break;

	case DCMD_IO_NET_PROMISCUOUS:
		if (*(int *)data) {
			at91sam->n_prom++;
			at91sam->cfg.flags |= NIC_FLAG_PROMISCUOUS;
		} else {
			if (at91sam->n_prom > 0) {
				if (--at91sam->n_prom == 0)
					at91sam->cfg.flags &= ~NIC_FLAG_PROMISCUOUS;
			}
		}

		status = at91sam_rx_set_mode(at91sam);
		break;

	case DCMD_IO_NET_CHANGE_MCAST:
		if (at91sam->cfg.flags & NIC_FLAG_MULTICAST)
			status = handle_multicast(at91sam, data);
		else
			status = ENOTSUP;
		break;

	default:
		status = ENOTSUP;
		break;
	}

	return status;
}
