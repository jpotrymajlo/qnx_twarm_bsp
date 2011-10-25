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

static char *at91sam_opts[] = {
	"receive",
	"transmit",
	"rmii",
	NULL
};

static int
at91sam_parse_options(at91sam_dev_t *at91sam,
    const char *optstring, nic_config_t *cfg)
{
	char		*value;
	int		opt;
	char		*options, *freeptr;
	char		*c;
	int		rc = 0;
	int		err = EOK;

	if (optstring == NULL)
		return 0;
	if (at91sam == NULL)
		return (-1);

	/* getsubopt() is destructive */
	freeptr = options = strdup(optstring);

	while (options && *options != '\0') {
		c = options;
		if ((opt = getsubopt(&options, at91sam_opts, &value)) == -1) {
			if (nic_parse_options(cfg, value) == EOK)
				continue;
			goto error;
		}

		switch (opt) {
			case 0:
				at91sam->num_rx_descriptors = strtoul(value, 0, 0);
				continue;
			case 1:
				continue;
			case 2:
				at91sam->rmii = AT91RM9200_USRIO_RMII;
				continue;
		}

error:
		nic_slogf(_SLOGC_NETWORK, _SLOG_WARNING, "devn-at91sam: unknown option %s", c);
		err = EINVAL;
		rc = -1;
	}

	free(freeptr);

	errno = err;

	return rc;
}

int
at91sam_detect(void *dll_hdl, io_net_self_t *ion, char *options)
{
	nic_config_t	*cfg;
	at91sam_dev_t	*at91sam;

	if ((at91sam = calloc(sizeof (*at91sam), 1)) == NULL)
		return -1;

	cfg = &at91sam->cfg;

	/* set some defaults for the command line options */
	cfg->flags      = NIC_FLAG_MULTICAST | NIC_FLAG_BROADCAST;
	cfg->media_rate = cfg->duplex = -1;
	cfg->priority   = NIC_PRIORITY;
	cfg->iftype     = IFT_ETHER;
	cfg->lan        = -1;
	strcpy(cfg->uptype, "en");
	at91sam->num_rx_descriptors = DEFAULT_NUM_RX_DESCRIPTORS;

	if (at91sam_parse_options(at91sam, options, cfg) == -1)
		return -1;

	if (at91sam->cfg.mtu == 0 || at91sam->cfg.mtu > ETH_MAX_PKT_LEN)
		at91sam->cfg.mtu = ETH_MAX_PKT_LEN;

	if (at91sam->cfg.mru == 0 || at91sam->cfg.mru > ETH_MAX_PKT_LEN)
		at91sam->cfg.mru = ETH_MAX_PKT_LEN;

	at91sam->force_advertise = -1;

	at91sam->cfg.revision = NIC_CONFIG_REVISION;
	at91sam->stats.revision = NIC_STATS_REVISION;
	at91sam->stats.valid_stats =
		    NIC_STAT_TXED_MULTICAST | NIC_STAT_RXED_MULTICAST |
		    NIC_STAT_TXED_BROADCAST | NIC_STAT_RXED_BROADCAST |
		    NIC_STAT_TX_FAILED_ALLOCS | NIC_STAT_RX_FAILED_ALLOCS;
	at91sam->stats.un.estats.valid_stats =
		    NIC_ETHER_STAT_INTERNAL_TX_ERRORS |
		    NIC_ETHER_STAT_INTERNAL_RX_ERRORS |
		    NIC_ETHER_STAT_NO_CARRIER |
		    NIC_ETHER_STAT_TX_DEFERRED |
		    NIC_ETHER_STAT_XCOLL_ABORTED |
		    NIC_ETHER_STAT_LATE_COLLISIONS |
		    NIC_ETHER_STAT_SINGLE_COLLISIONS |
		    NIC_ETHER_STAT_MULTI_COLLISIONS |
		    NIC_ETHER_STAT_TOTAL_COLLISION_FRAMES |
		    NIC_ETHER_STAT_ALIGN_ERRORS |
		    NIC_ETHER_STAT_FCS_ERRORS |
		    NIC_ETHER_STAT_SHORT_PACKETS;

	if (cfg->duplex != -1) {
		/* User forced the duplex, in this case, we disable autoneg. */
		at91sam->force_advertise = 0;
		if (cfg->media_rate == -1) {
			/* Force speed to a default */
			cfg->media_rate = 100 * 1000;
		}
	}
	if (cfg->media_rate != -1) {
		if (cfg->media_rate == 100 * 1000)
			at91sam->force_advertise = (cfg->duplex == 1) ? MDI_100bTFD : MDI_100bT;
		else
			at91sam->force_advertise = (cfg->duplex == 1) ? MDI_10bTFD : MDI_10bT;
	}

	if (cfg->num_irqs == 0) {
		cfg->irq[0] = 24;
		cfg->num_irqs = 1;
	}

	if (cfg->num_io_windows == 0) {
		cfg->io_window_base[0] = AT91RM9200_EMAC_BASE;
		cfg->num_io_windows = 1;
	}

	if (!at91sam_register_device(at91sam, ion, dll_hdl)) {
		at91sam_advertise(at91sam->reg_hdl, at91sam);
		return (EOK);
	}

	return -1;
}

