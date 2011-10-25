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




#ifndef __AT91RM9200_H__
#define __AT91RM9200_H__


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gulliver.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <sys/mman.h>
#include <sys/dcmd_io-net.h>
#include <sys/io-net.h>
#include <sys/slogcodes.h>
#include <sys/cache.h>
#include <hw/inout.h>
#include <drvr/mdi.h>
#include <drvr/eth.h>
#include <drvr/nicsupport.h>

#include <arm/at91rm9200.h>

#include <_pack1.h>

/* Transmit and Receive buffer descriptor */
typedef struct BD {
    uint32_t    addr;   /* Base address of receive buffer */
    uint32_t    size;   /* Receive length and status */
} BD_t;

#define AT91RM9200_INTERRUPT_EVENT      (MDI_TIMER + 1)

#define NIC_PRIORITY                21
#define NIC_MDI_PRIORITY            10

#define DEFAULT_NUM_RX_DESCRIPTORS  32 * 12

#define AT91RM9200_RBD_DONE             (1 << 0)
#define AT91RM9200_RBD_WRAP             (1 << 1)
#define AT91RM9200_RBD_SOF              (1 << 14)
#define AT91RM9200_RBD_EOF              (1 << 15)

#define AT91RM9200_TBD_FREE             (1 << 31)


#define AT91RM9200_RBD_BCAST    (1 << 31)
#define AT91RM9200_RBD_MCAST    (1 << 30)
#define AT91RM9200_RBD_UCAST    (1 << 29)
#define AT91RM9200_RBD_LEN_MASK 0x7FF

#define AT91RM9200_RX_BUF_SIZE          128
#define AT91RM9200_MAX_FRAME_LEN        1520
#define	AT91RM9200_DEFRAG_PACKET		 0x1000000

typedef struct _nic_at91sam_ext {
    nic_config_t        cfg;
    nic_stats_t         stats;
    int                 chid;       /* channel id */
    int                 coid;       /* connection id */
    int                 tid;        /* thread id */
    int                 iid;        /* interrupt id */
    struct cache_ctrl   cachectl;
    io_net_self_t       *ion;
    void                *dll_hdl;
    int                 reg_hdl;
    uint16_t            cell;
    uint16_t            _align;

    uint32_t            iobase;

    /* Rx */
    uint32_t            rbds_buf_vbase;
    uint32_t            rbds_buf_pbase;
    BD_t                *rbds_base;
    BD_t                *rbds;
    npkt_t              **rx_pkts;
    int                 rx_rptr;
    npkt_t              *rx_free_pkt_q;
    pthread_mutex_t     rx_pkt_q_mutex;
    unsigned            num_rx_free;
    int                 num_rx_descriptors;

    /* Tx */
    npkt_t              *nhead;
    npkt_t              *ntail;
    npkt_t              *current_frame;
    pthread_mutex_t     tx_mutex;

    /* Mii */
    mdi_t               *mdi;
    int                 rmii;
    int                 pkts_received;      // optimization to not probe phy
    int                 force_advertise;

    /* Multicast/promiscuous tracking */
    uint32_t            hash[2];    /* Hash address */
    uint32_t            hcnt[64];
    unsigned            n_allmulti;
    unsigned            n_prom;

    /* Statistics */
    int                 n_intr;
} at91sam_dev_t;

/* at91sam.c */
int at91sam_init(void *dll_hdl, dispatch_t *dpp, io_net_self_t *ion, char *options);
npkt_t *at91sam_alloc_npkt(at91sam_dev_t *dev);
int at91sam_send_bcmd(at91sam_dev_t *dev, uint8_t bcmd, int wait);
int at91sam_send_intmask(at91sam_dev_t *dev, uint8_t intmask);
int at91sam_rx_set_mode(at91sam_dev_t *dev);
int at91sam_register_device(at91sam_dev_t *dev, io_net_self_t *ion, void *dll_hdl);
int at91sam_detect(void *dll_hdl, io_net_self_t *ion, char *options);
int at91sam_advertise(int reg_hdl, void *func_hdl);
int at91sam_flush(int reg_hdl, void *func_hdl);

/* devctl.c */
int at91sam_update_stats(at91sam_dev_t *at91sam);
int at91sam_devctl(void *hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret);

/* transmit.c */
int at91sam_send_packets(npkt_t *npkt, void *rx_down_hdl);
int at91sam_transmit_complete(at91sam_dev_t *dev);

/* receive.c */
void at91sam_receive(at91sam_dev_t *dev);
int at91sam_receive_complete(npkt_t *npkt, void *done_hdl, void *func_hdl);

/* mii.c */
int at91sam_init_phy(at91sam_dev_t *dev);

/* shutdown.c */
int at91sam_shutdown1(int reg_hdl, void *func_hdl);
int at91sam_shutdown2(int reg_hdl, void *func_hdl);

#define AT91RM9200_IS_BROADCAST(dptr) \
        ((dptr)[0] == 0xff && (dptr)[1] == 0xff && \
        (dptr)[2] == 0xff && (dptr)[3] == 0xff && \
        (dptr)[4] == 0xff && (dptr)[5] == 0xff)

#endif
