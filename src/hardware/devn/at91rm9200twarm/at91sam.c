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

io_net_dll_entry_t io_net_dll_entry = {
	2,
	at91sam_init,
	NULL
};

io_net_registrant_funcs_t	at91sam_funcs = {
	8,
	NULL,						/* rx_up  - I'm a driver */
	&at91sam_send_packets,		/* rx_down() */
	&at91sam_receive_complete,	/* tx_done() */
	&at91sam_shutdown1,			/* shutdown() */
	&at91sam_shutdown2,			/* shutdown() */
	&at91sam_advertise,			/* advertise_ifaces ??? */
	&at91sam_devctl,			/* devctl() */
	&at91sam_flush,				/* flush() */
	NULL						/* RAW open() ??? - nraw */
};

io_net_registrant_t at91sam_entry = {
	_REG_PRODUCER_UP | _REG_TRACK_MCAST,		/* flags */
	"devn-at91sam",			/* name */
	"en",					/* top_type */
	NULL,					/* bot_type */
	NULL,					/* rx_down_hdl - loaded on each register */
	&at91sam_funcs,			/* funcs */
							/* dependencies */
};

void* at91sam_event_handler(at91sam_dev_t *at91sam);

/*
 * at91sam_init()
 *
 * Description: main entry point for the driver
 *
 * Returns: 0 if OK else -1 on error with errno set
 *
 */
int
at91sam_init(void *dll_hdl, dispatch_t *dpp, io_net_self_t *ion, char *options)
{
	int ret;

	/* ARGSUSED */
	dpp = dpp;

	if (ret = at91sam_detect(dll_hdl, ion, options)) {
		errno = ret;
		return -1;
	}

	return 0;
}

static int
at91sam_init_ring(at91sam_dev_t *at91sam)
{
	int			cnt;
    char        *ptr;
	npkt_t		*npkt = NULL;
	BD_t		*rbd;
	net_iov_t	*net_iov;
    uint32_t    phy_addr;

    /* Initialize RX buffer descriptor list */
	at91sam->rbds_base = mmap(0, sizeof(BD_t) * at91sam->num_rx_descriptors,
	    PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_ANON|MAP_PHYS, NOFD, 0);

	if (at91sam->rbds_base == MAP_FAILED)
		return -1;

    /* Set base address for bds */
    rbd = at91sam->rbds = at91sam->rbds_base;

	at91sam->rx_pkts = calloc(sizeof(npkt_t *), at91sam->num_rx_descriptors);
	if (at91sam->rx_pkts == NULL)
		goto fail1;

    /*
	 * Allocate memory for buffer descriptor and pad it.
	 */
    ptr = mmap( 0, AT91RM9200_RX_BUF_SIZE * at91sam->num_rx_descriptors
	          , PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_ANON|MAP_PHYS, NOFD, 0);

    if (ptr == MAP_FAILED) {
		at91sam->ion->free(at91sam->rx_pkts);
		return NULL;
	}

    /* Keep virtual address of ptr */
    at91sam->rbds_buf_vbase = (uint32_t)ptr;
    at91sam->rbds_buf_pbase = phy_addr = (uint32_t)drvr_mphys(ptr);

	/* Init rbd's  */
	for (cnt = 0; cnt < at91sam->num_rx_descriptors; cnt++) {

		rbd->size = AT91RM9200_TBD_FREE;
		rbd->addr = phy_addr;

        if (cnt == at91sam->num_rx_descriptors - 1)
            rbd->addr |= AT91RM9200_RBD_WRAP;

        /* Move data for next BD */
        rbd++;
        phy_addr += AT91RM9200_RX_BUF_SIZE;
        ptr += AT91RM9200_RX_BUF_SIZE;

	}

	/* Allocate pool of buffer */
	for (cnt = 0; cnt < at91sam->num_rx_descriptors; cnt++) {
		if ((npkt = at91sam_alloc_npkt(at91sam)) == NULL)
			goto fail2;

		npkt->flags = _NPKT_UP;

		net_iov = npkt->buffers.tqh_first->net_iov;

		at91sam->rx_pkts[cnt] = npkt;
	}

	at91sam->current_frame = NULL;


	return 0;

fail2:
	for (cnt = 0; cnt < at91sam->num_rx_descriptors; cnt++)
		if (at91sam->rx_pkts[cnt] != NULL)
			at91sam->ion->free(at91sam->rx_pkts[cnt]);
	free(at91sam->rx_pkts);

	free(at91sam->rx_pkts);


fail1:
	munmap(at91sam->rbds_base, sizeof(BD_t) * at91sam->num_rx_descriptors);

	return -1;
}


int at91sam_rx_set_mode(at91sam_dev_t *at91sam)
{
	uint32_t	cfg;
	uintptr_t	iobase = at91sam->iobase;

	pthread_mutex_lock(&at91sam->tx_mutex);

	cfg = in32(iobase + AT91RM9200_CFG);
	cfg &= ~(AT91RM9200_CFG_CAF | AT91RM9200_CFG_MTI | AT91RM9200_CFG_NBC | AT91RM9200_CFG_SPD100 | AT91RM9200_CFG_FD);

	if (at91sam->cfg.flags & NIC_FLAG_PROMISCUOUS)
		cfg |= AT91RM9200_CFG_CAF;

	if (at91sam->cfg.flags & NIC_FLAG_MULTICAST) {
		cfg |= AT91RM9200_CFG_MTI;
		if (at91sam->n_allmulti) {
			out32(iobase + AT91RM9200_HSL, 0xFFFFFFFF);
			out32(iobase + AT91RM9200_HSH, 0xFFFFFFFF);
		} else {
			out32(iobase + AT91RM9200_HSL, at91sam->hash[0]);
			out32(iobase + AT91RM9200_HSH, at91sam->hash[1]);
		}
	}

	if (!(at91sam->cfg.flags & NIC_FLAG_BROADCAST))
		cfg |= AT91RM9200_CFG_NBC;

	if (at91sam->cfg.duplex)
		cfg |= AT91RM9200_CFG_FD;

	if (at91sam->cfg.media_rate == 100 * 1000)
		cfg |= AT91RM9200_CFG_SPD100;

	out32(iobase + AT91RM9200_CFG, cfg);

	pthread_mutex_unlock(&at91sam->tx_mutex);

	return 0;
}


static int
at91sam_reset(at91sam_dev_t *at91sam)
{
	int			i;
	uintptr_t	iobase = at91sam->iobase;
	uint8_t		*ptr_mac = at91sam->cfg.current_address;

	/* Disable all interrupts */
	out32(iobase + AT91RM9200_IDR, 0x00003CFF);

	/* Disable Tx/Rx */
	out32(iobase + AT91RM9200_CTL, 0);

	if (at91sam_init_ring(at91sam) == -1)
		return -1;

	/*
	 * Program MAC address at SA1, clear all SA registers
	 */
	out32(iobase + AT91RM9200_SA1L, *(uint32_t *)&ptr_mac[0]);
	out32(iobase + AT91RM9200_SA1H, *(uint16_t *)&ptr_mac[4]);
	for (i = 1; i < 4; i++) {
		out32(iobase + AT91RM9200_SA1L + (i * 8), 0);
		out32(iobase + AT91RM9200_SA1H + (i * 8), 0);
	}

	/* Load rx base address */
	out32(iobase + AT91RM9200_RBQP, drvr_mphys(at91sam->rbds_base));

	/* Load tx base address */
//	out32(iobase + AT91RM9200_TBQP, drvr_mphys(at91sam->tbds_base));

	/* Clear all statistics registers */
	for (i = AT91RM9200_FRA; i <= AT91RM9200_SQEE; i += 4)
		in32(iobase + i);

	/* Enable all interrupts, except... */
	out32(iobase + AT91RM9200_IER, AT91RM9200_RCOM | AT91RM9200_RXUBR | AT91RM9200_TUND |
					AT91RM9200_RTRY | AT91RM9200_TCOM | AT91RM9200_ROVR);

	if (at91sam->cfg.flags & NIC_FLAG_MULTICAST)
		at91sam_rx_set_mode(at91sam);

	return 0;
}

static int
at91sam_config(at91sam_dev_t *at91sam)
{
	uintptr_t	iobase;
	const char	zaddr[6] = {0,};
	uint8_t		*pmac = (uint8_t *)at91sam->cfg.permanent_address;

	iobase = mmap_device_io(AT91RM9200_EMAC_SIZE,
					at91sam->cfg.io_window_base[0]);

	if (iobase == (uintptr_t)MAP_FAILED) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR,
					"devn-at91sam: Unable to mmap_device_io.");
		return ENODEV;
	}

	at91sam->iobase = iobase;

	strcpy(at91sam->cfg.device_description, "ATMEL AT91 EMAC");

	if (!at91sam->num_rx_descriptors)
		at91sam->num_rx_descriptors	= DEFAULT_NUM_RX_DESCRIPTORS;

	at91sam->num_rx_free = 0;

	/* Override with MAC address from syspage, if there is one */
	nic_get_syspage_mac(pmac);

	/* Read MAC address from SA */
	if (memcmp(pmac, zaddr, ETH_MAC_LEN) == 0) {
		int		i, sal, sah;

		for (i = 0; i < 4; i++) {
			sal = in32(iobase + AT91RM9200_SA1L + (i * 8));
			sah = in32(iobase + AT91RM9200_SA1H + (i * 8));

			/*
			 * TODO : Check if the MAC address is valid
			 */
			if (sal != 0 || sah != 0) {
				*(uint32_t *)&pmac[0] = sal;
				*(uint16_t *)&pmac[4] = sah;
				break;
			}
		}
	}

	/* Check for command line override */
	if (memcmp(at91sam->cfg.current_address, zaddr, ETH_MAC_LEN) == 0) {
		if (memcmp(pmac, zaddr, ETH_MAC_LEN) == 0) {
			nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR, "devn-at91sam: MAC address not specified on cmdline.");
			munmap_device_io(iobase, AT91RM9200_EMAC_SIZE);
			return -1;
		}
		memcpy(at91sam->cfg.current_address, pmac, ETH_MAC_LEN);
	}

	at91sam->cfg.media = NIC_MEDIA_802_3;
	at91sam->cfg.mac_length = ETH_MAC_LEN;

	if (at91sam->cfg.verbose)
		nic_dump_config(&at91sam->cfg);

	return(0);
}

static int
at91sam_initialize(at91sam_dev_t *at91sam)
{
	at91sam->cfg.connector = NIC_CONNECTOR_MII;

	out32(at91sam->iobase + AT91RM9200_CFG,   AT91RM9200_CFG_BIG);
//    out32(at91sam->iobase + AT91RM9200_USRIO, at91sam->rmii | AT91RM9200_USRIO_CLKEN);

	if (at91sam_init_phy(at91sam) == -1)
		return -1;

	if (at91sam_reset(at91sam) == -1)
		return -1;
	return 0;
}

int
at91sam_register_device(at91sam_dev_t *at91sam, io_net_self_t *ion, void *dll_hdl)
{
	pthread_attr_t		pattr;
	struct sched_param	param;
	struct sigevent		event;
	uint16_t			lan;

	at91sam->ion = ion;
	at91sam->dll_hdl = dll_hdl;

	if ((at91sam->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1)
		return -1;
	if ((at91sam->coid = ConnectAttach(0, 0, at91sam->chid, _NTO_SIDE_CHANNEL, 0)) == -1)
		goto fail1;
	if (pthread_mutex_init(&at91sam->tx_mutex, NULL) == -1)
		goto fail2;
	if (pthread_mutex_init(&at91sam->rx_pkt_q_mutex, NULL) == -1)
		goto fail3;

	at91sam->cachectl.fd = NOFD;

	if (cache_init(0, &at91sam->cachectl, NULL) == -1)
		goto fail4;

	pthread_attr_init(&pattr);
	pthread_attr_setschedpolicy(&pattr, SCHED_RR);
	param.sched_priority = at91sam->cfg.priority;
	pthread_attr_setschedparam(&pattr, &param);
	pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

	if (at91sam_config(at91sam))
		goto fail5;

	if (at91sam_initialize(at91sam) == -1)
		goto fail6;

	if (pthread_create(&at91sam->tid, NULL, (void *)at91sam_event_handler, at91sam))
		goto fail6;

	at91sam_entry.func_hdl = (void *)(at91sam);
	at91sam_entry.top_type = at91sam->cfg.uptype;

	if (at91sam->cfg.lan != -1) {
		at91sam_entry.flags |= _REG_ENDPOINT_ARG;
		lan = at91sam->cfg.lan;
	}

	if (at91sam->ion->reg((void *)dll_hdl,
	    &at91sam_entry, &at91sam->reg_hdl, &at91sam->cell, &lan) < 0)
		goto fail7;

	at91sam->cfg.lan = lan;

	/* Create the interface thread */
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = at91sam->coid;
	event.sigev_code = AT91RM9200_INTERRUPT_EVENT;
	event.sigev_priority = at91sam->cfg.priority;

	if ((at91sam->iid = InterruptAttachEvent(at91sam->cfg.irq[0],
									&event, _NTO_INTR_FLAGS_TRK_MSK)) == -1)
		goto fail7;

	/* Enable Rx/Tx */
	out32(at91sam->iobase + AT91RM9200_CTL, AT91RM9200_CTL_RE | AT91RM9200_CTL_TE);

	return 0;

fail7:
	pthread_cancel(at91sam->tid);
	pthread_join(at91sam->tid, NULL);
fail6:
	munmap_device_io(at91sam->iobase, AT91RM9200_EMAC_SIZE);
fail5:
	cache_fini(&at91sam->cachectl);
fail4:
	pthread_mutex_destroy(&at91sam->rx_pkt_q_mutex);
fail3:
	pthread_mutex_destroy(&at91sam->tx_mutex);
fail2:
	ConnectDetach(at91sam->coid);
fail1:
	ChannelDestroy(at91sam->chid);

	return -1;
}

/*
 * at91sam_process_interrupt()
 *
 * Description: Handle all interrupts for the device
 *
 * Returns: N/A
 */
static void
at91sam_process_interrupt(at91sam_dev_t *at91sam)
{
	uint32_t	stat, ctrl;
	uintptr_t	iobase = at91sam->iobase;
    stat = in32(iobase + AT91RM9200_ISR);

    if (stat & AT91RM9200_RCOM) {	/* Receive complete */
    	at91sam_receive(at91sam);
        at91sam->pkts_received = 1;
    }
    if (stat & AT91RM9200_TUND) {
        if (at91sam->cfg.verbose) {
            nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: Attempt to recover from underrun!");
        }

        /* Need to reset transmitter */
        at91sam_flush(at91sam->reg_hdl, at91sam);
    }
    else if (stat & AT91RM9200_TCOM)
        at91sam_transmit_complete(at91sam);

    if (stat & AT91RM9200_RXUBR) {
        ctrl = in32(iobase + AT91RM9200_RSR);

        if (ctrl & AT91RM9200_RSR_BNA) {
            ctrl = in32(iobase + AT91RM9200_CTL);

            out32(iobase + AT91RM9200_CTL, ctrl & ~AT91RM9200_CTL_RE);
            out32(iobase + AT91RM9200_CTL, ctrl | AT91RM9200_CTL_RE);
        }
    }

    /* Staticstics update */
    if (++at91sam->n_intr > 64) {
        at91sam_update_stats(at91sam);
        at91sam->n_intr = 0;
    }

	InterruptUnmask(at91sam->cfg.irq[0], at91sam->iid);
}

void *
at91sam_event_handler(at91sam_dev_t *at91sam)
{
	struct _pulse		pulse;
	iov_t			iov;
	int			rcvid;

	SETIOV(&iov, &pulse, sizeof(pulse));

	while (1) {
		if ((rcvid = MsgReceivev(at91sam->chid, &iov, 1, NULL)) == -1) {
			if (errno == ESRCH) {
				pthread_exit(NULL);
			}
			continue;
		}

		switch (pulse.code) {
		case AT91RM9200_INTERRUPT_EVENT:
			at91sam_process_interrupt(at91sam);
			break;

		case MDI_TIMER:
			// Only monitor the link if:
			//   1) the link state is unknown, or
			//   2) there's no traffic.
			if ((at91sam->cfg.flags & NIC_FLAG_LINK_DOWN) ||
			   (at91sam->cfg.media_rate <= 0) || !at91sam->pkts_received) {
			    MDI_MonitorPhy(at91sam->mdi);
			}
			at91sam->pkts_received = 0;
			break;
		default:
			if (rcvid)
				MsgReplyv(rcvid, ENOTSUP, &iov, 1);
			break;
		}
	}

	return NULL;
}

int
at91sam_flush(int reg_hdl, void *func_hdl)
{
	at91sam_dev_t	*at91sam = (at91sam_dev_t *)func_hdl;
    uintptr_t	    iobase = at91sam->iobase;
	npkt_t			*npkt, *tmp;
    unsigned        ctl;

	pthread_mutex_lock(&at91sam->tx_mutex);
	/* Disable all interrupts */
	out32(iobase + AT91RM9200_IDR, 0x00003CFF);

	/* Disable Tx */
    ctl = in32(iobase + AT91RM9200_CTL);
	out32(iobase + AT91RM9200_CTL, ctl & ~ AT91RM9200_CTL_TE);

    /* Clear Tx Status */
    out32(iobase + AT91RM9200_TSR, in32(iobase + AT91RM9200_TSR));

	npkt = at91sam->nhead;
	at91sam->nhead = at91sam->ntail = NULL;

	for( ; npkt; npkt = tmp) {
		tmp = npkt->next;
        at91sam->ion->free(npkt->org_data);
		at91sam->ion->free(npkt);
	}
	/* Enable all interrupts, except... */
	out32(iobase + AT91RM9200_IER, AT91RM9200_RCOM | AT91RM9200_RXUBR | AT91RM9200_TUND |
					AT91RM9200_RTRY | AT91RM9200_TCOM | AT91RM9200_ROVR);

    /* Enable Tx */
	out32(at91sam->iobase + AT91RM9200_CTL, AT91RM9200_CTL_RE | AT91RM9200_CTL_TE);
	pthread_mutex_unlock(&at91sam->tx_mutex);
	return 0;
}

npkt_t *
at91sam_alloc_npkt(at91sam_dev_t *at91sam)
{
	npkt_t			*npkt;
	net_buf_t		*nb;
	net_iov_t		*iov;
	char			*ptr;
	int			linesize = at91sam->cachectl.cache_line_size;

	if ((npkt = at91sam->ion->alloc_up_npkt(sizeof(net_buf_t) +
	    sizeof(net_iov_t), (void **)&nb)) == NULL)
		return NULL;

	/*
	 * Make sure the buffer does not share cache lines with any
	 * other data, so that when we invalidate cache lines associated
	 * with the buffer, we don't corrupt anything.
	 */
	if ((ptr = at91sam->ion->alloc(AT91RM9200_MAX_FRAME_LEN + linesize * 2, 0)) == NULL) {
		at91sam->ion->free(npkt);
		return NULL;
	}

	npkt->org_data = ptr;

	/* Pad buffer to begin on a cache line boundary */
	if (linesize != 0)
		ptr = (char *)((unsigned)(ptr + (linesize - 1)) & ~(linesize - 1));

	TAILQ_INSERT_HEAD(&npkt->buffers, nb, ptrs);

	iov = (net_iov_t *)(nb + 1);

	nb->niov = 1;
	nb->net_iov = iov;

	iov->iov_base = (void *)ptr;
	iov->iov_len = AT91RM9200_MAX_FRAME_LEN;
	iov->iov_phys = (paddr_t)at91sam->ion->mphys(iov->iov_base);

	npkt->next = NULL;
	npkt->tot_iov = 1;
	npkt->ref_cnt = 1;
	npkt->req_complete = 0;

	return npkt;
}

int
at91sam_advertise(int reg_hdl, void *func_hdl)
{
	npkt_t			*npkt;
	net_buf_t		*nb;
	net_iov_t		*iov;
	at91sam_dev_t		*at91sam = func_hdl;
	io_net_msg_dl_advert_t	*ap;

	if ((npkt = at91sam_alloc_npkt(at91sam)) == NULL)
		return 0;

	nb = npkt->buffers.tqh_first;
	iov = nb->net_iov;

	/* realign to dword (have to undo this in at91sam_receive_complete()). */
	iov->iov_base = (void*)((int)iov->iov_base & ~0x2);
	ap = iov->iov_base;
	iov->iov_len = sizeof *ap; //reset this

	memset(ap, 0x00, sizeof *ap);
	ap->type   = _IO_NET_MSG_DL_ADVERT;

	ap->iflags = (IFF_SIMPLEX | IFF_BROADCAST | IFF_RUNNING);
	if (at91sam->cfg.flags & NIC_FLAG_MULTICAST)
		ap->iflags |= IFF_MULTICAST;

	ap->mtu_min       = 0;
	ap->mtu_max       = at91sam->cfg.mtu;
	ap->mtu_preferred = at91sam->cfg.mtu;

	strcpy(ap->up_type, at91sam->cfg.uptype);
	itoa(at91sam->cfg.lan, ap->up_type + strlen(ap->up_type), 10);

	strcpy(ap->dl.sdl_data, ap->up_type);

	ap->dl.sdl_len = sizeof(struct sockaddr_dl);
	ap->dl.sdl_family = AF_LINK;
	ap->dl.sdl_index  = at91sam->cfg.lan;
	ap->dl.sdl_type = at91sam->cfg.iftype;
	ap->dl.sdl_nlen = strlen(ap->dl.sdl_data); // not null terminated
	ap->dl.sdl_alen = 6;
	memcpy(ap->dl.sdl_data + ap->dl.sdl_nlen, at91sam->cfg.current_address, 6);

	npkt->flags |= _NPKT_MSG;

	npkt->iface = 0;
	npkt->framelen = sizeof *ap;

	if (at91sam->ion->reg_tx_done(at91sam->reg_hdl, npkt, at91sam) == -1) {
		at91sam->ion->free(npkt->org_data);
		at91sam->ion->free(npkt);
		return(0);
	}

	if (at91sam->ion->tx_up(at91sam->reg_hdl, npkt, 0, 0, at91sam->cell, at91sam->cfg.lan, 0) == 0)
		at91sam->ion->tx_done(at91sam->reg_hdl, npkt);

	return 0;
}
