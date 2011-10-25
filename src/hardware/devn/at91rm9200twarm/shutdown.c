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

int
at91sam_shutdown1(int reg_hdl, void *func_hdl)
{
	at91sam_dev_t	*at91sam = func_hdl;

	if (at91sam->cfg.verbose)
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
		    "devn-at91sam: Starting Shutdown1(hdl=%08x)",
		    (uint32_t)reg_hdl);

	if (reg_hdl != at91sam->reg_hdl) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR,
		    "devn-at91sam: Internal Error: driver handle mismatch.");
		return(EFAULT);
	}

	/* Disable all interrupts */
	out32(at91sam->iobase + AT91RM9200_IDR, 0x00000FFF);

	if (at91sam->cfg.verbose)
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: Finished Shutdown1(hdl=%08x)", (uint32_t)reg_hdl);

	return(EOK);
}

int
at91sam_shutdown2(int reg_hdl, void *func_hdl)
{
	at91sam_dev_t	*at91sam = func_hdl;
	npkt_t			*npkt;
	int				i;

	if (at91sam->cfg.verbose)
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: Starting Shutdown2(hdl=%08x)", (uint32_t)reg_hdl);

	/* Disable all interrupts */
	out32(at91sam->iobase + AT91RM9200_IDR, 0x00000FFF);

	InterruptDetach(at91sam->iid);
	ConnectDetach(at91sam->coid);
	ChannelDestroy(at91sam->chid);

	/* Disable Tx/Rx */
	out32(at91sam->iobase + AT91RM9200_CTL, 0);

	if (at91sam->mdi != NULL)
		MDI_DeRegister(&at91sam->mdi);

	pthread_cancel(at91sam->tid);
	pthread_join(at91sam->tid, NULL);

	for (i = 0; i < at91sam->num_rx_descriptors; i++)
		if (at91sam->rx_pkts[i] != NULL)
			at91sam->ion->free(at91sam->rx_pkts[i]);
	free(at91sam->rx_pkts);
	munmap(at91sam->rbds, sizeof(BD_t) * at91sam->num_rx_descriptors);

	for(; npkt = at91sam->rx_free_pkt_q;) {
		at91sam->rx_free_pkt_q = npkt->next;
		at91sam->ion->free(npkt->org_data);
		at91sam->ion->free(npkt);
	}

	at91sam_flush(at91sam->reg_hdl, at91sam);

	munmap_device_io(at91sam->iobase, AT91RM9200_EMAC_SIZE);
	cache_fini(&at91sam->cachectl);

	pthread_mutex_destroy(&at91sam->rx_pkt_q_mutex);
	pthread_mutex_destroy(&at91sam->tx_mutex);
	free(at91sam);

	if (at91sam->cfg.verbose)
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: Finished Shutdown2(hdl=%08x)", (uint32_t)reg_hdl);

	return 0;
}
