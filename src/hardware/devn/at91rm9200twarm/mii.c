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


#define	AT91RM9200_PHY_READ(p, r)		((1 << 30) | (2 << 28) | (2 << 16) | ((p & 0x1f) << 23) | (r << 18))
#define	AT91RM9200_PHY_WRITE(p, r, d)	((1 << 30) | (1 << 28) | (2 << 16) | ((p & 0x1f) << 23) | (r << 18) | ((d) & 0xffff))

uint16_t
at91sam_mii_read(void *handle, uchar_t phy_id, uint8_t location)
{
	at91sam_dev_t	*at91sam = handle;
	uintptr_t		base = at91sam->iobase;
	uint32_t		ctrl, cnt, data = 0;

	ctrl = in32(base + AT91RM9200_CTL);
	out32(base + AT91RM9200_CTL, ctrl | AT91RM9200_CTL_MPE);

	/* Issue PHY read command */
	out32(base + AT91RM9200_MAN, AT91RM9200_PHY_READ(phy_id, location));

	for (cnt = 0; cnt < 256; cnt++) {
		nsec_delay(30 * 1000);
		if (in32(base + AT91RM9200_SR) & AT91RM9200_SR_IDLE) {
			data = in32(base + AT91RM9200_MAN) & 0xFFFF;
			break;
		}
	}

	out32(base + AT91RM9200_CTL, ctrl);

	return data;
}

void
at91sam_mii_write(void *handle, uchar_t phy_id, uint8_t location, uint16_t value)
{
	at91sam_dev_t	*at91sam = handle;
	uintptr_t		base = at91sam->iobase;
	uint32_t		ctrl, cnt;

	ctrl = in32(base + AT91RM9200_CTL);
	out32(base + AT91RM9200_CTL, ctrl | AT91RM9200_CTL_MPE);

	/* Issue PHY write command */
	out32(base + AT91RM9200_MAN, AT91RM9200_PHY_WRITE(phy_id, location, value));

	for (cnt = 0; cnt < 256; cnt++) {
		nsec_delay(30 * 1000);
		if (in32(base + AT91RM9200_SR) & AT91RM9200_SR_IDLE)
			break;
	}

	out32(base + AT91RM9200_CTL, ctrl);
}

void
at91sam_mii_callback(void *handle, uchar_t phy, uchar_t newstate)
{
	at91sam_dev_t	*at91sam = handle;
	int				sts, mode;
	char			*s;

	switch (newstate) {
	case MDI_LINK_UP:
		if ((sts = MDI_GetActiveMedia(at91sam->mdi, at91sam->cfg.phy_addr, &mode)) != MDI_LINK_UP) {
			if (at91sam->cfg.verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: callback GetActiveMedia returned %x", sts);
			mode = 0;
		}

		switch (mode) {
		case MDI_10bTFD:
			s = "10 BaseT Full Duplex";
			at91sam->cfg.duplex = 1;
			at91sam->cfg.media_rate = 10 * 1000L;
			break;
		case MDI_10bT:
			s = "10 BaseT Half Duplex";
			at91sam->cfg.duplex = 0;
			at91sam->cfg.media_rate = 10 * 1000L;
			break;
		case MDI_100bTFD:
			s = "100 BaseT Full Duplex";
			at91sam->cfg.duplex = 1;
			at91sam->cfg.media_rate = 100 * 1000L;
			break;
		case MDI_100bT:
			s = "100 BaseT Half Duplex";
			at91sam->cfg.duplex = 0;
			at91sam->cfg.media_rate = 100 * 1000L;
			break;
		case MDI_100bT4:
			s = "100 BaseT4";
			at91sam->cfg.duplex = 0;
			at91sam->cfg.media_rate = 100 * 1000L;
			break;
		default:
			s = "Unknown";
			at91sam->cfg.duplex = 0;
			at91sam->cfg.media_rate = 0L;
			break;
		}

		if (at91sam->cfg.verbose)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO, "devn-at91sam: link up (%s)", s);

		at91sam->cfg.flags &= ~NIC_FLAG_LINK_DOWN;

		at91sam_rx_set_mode(at91sam);

		break;
	case MDI_LINK_DOWN:
		at91sam->cfg.media_rate = at91sam->cfg.duplex = -1;
		at91sam->cfg.flags |= NIC_FLAG_LINK_DOWN;

		if (at91sam->cfg.verbose)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
			    "Link down %d", at91sam->cfg.lan);

		MDI_AutoNegotiate(at91sam->mdi, at91sam->cfg.phy_addr, NoWait);

		break;
	default:
		break;
	}
}

int
at91sam_init_phy(at91sam_dev_t *at91sam)
{
	struct sigevent		mdi_event;
	int					status, an_capable;
	uint16_t			reg;

	mdi_event.sigev_coid = at91sam->coid;
	status = MDI_Register_Extended (at91sam, at91sam_mii_write, at91sam_mii_read,
	    at91sam_mii_callback, (mdi_t **)&at91sam->mdi, &mdi_event,
	    NIC_MDI_PRIORITY, 3); /* 3 second interval */

	if (status != MDI_SUCCESS) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR,
		    "devn-at91sam: Cannot register the mii routines");
		at91sam->mdi = NULL;
		return -1;
	}

	for (at91sam->cfg.phy_addr = 0;
	    at91sam->cfg.phy_addr < 32; at91sam->cfg.phy_addr++) {
		if (MDI_FindPhy(at91sam->mdi, at91sam->cfg.phy_addr) == MDI_SUCCESS)
			break;
	}

	if (at91sam->cfg.phy_addr == 32) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
		    "devn-at91sam: Cannot find an active PHY");
		return -1;
	}

	if (at91sam->cfg.verbose)
		nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
		    "devn-at91sam: MII transceiver found at address %d.",
		    at91sam->cfg.phy_addr);

	if (MDI_InitPhy(at91sam->mdi, at91sam->cfg.phy_addr) != MDI_SUCCESS) {
		nic_slogf(_SLOGC_NETWORK, _SLOG_ERROR,
		    "devn-at91sam: Cannot init the PHY status");
		return -1;
	}

    /* Reset PHY */
    at91sam_mii_write(at91sam, at91sam->cfg.phy_addr, MDI_BMCR, BMCR_RESET);
    while(at91sam_mii_read(at91sam, at91sam->cfg.phy_addr, MDI_BMCR) & BMCR_RESET);

	an_capable = at91sam_mii_read(at91sam, at91sam->cfg.phy_addr, MDI_BMSR) & BMSR_AN_ABILITY;

	if (at91sam->force_advertise != -1 || !an_capable) {
		reg = at91sam_mii_read(at91sam, at91sam->cfg.phy_addr, MDI_BMCR);

		reg &= ~(BMCR_RESTART_AN | BMCR_SPEED_100 | BMCR_FULL_DUPLEX);

		if (an_capable && at91sam->force_advertise != 0) {
			/*
			 * If we force the speed, but the link partner
			 * is autonegotiating, there is a greater chance
			 * that everything will work if we advertise with
			 * the speed that we are forcing to.
			 */
			MDI_SetAdvert(at91sam->mdi,
			    at91sam->cfg.phy_addr, at91sam->force_advertise);

			reg |= BMCR_RESTART_AN | BMCR_AN_ENABLE;

			if (at91sam->cfg.verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
				    "devn-at91sam: "
				    "restricted autonegotiate (%dMbps only)",
				    at91sam->cfg.media_rate/1000);
		} else {
			reg &= ~BMCR_AN_ENABLE;

			if (at91sam->cfg.verbose)
				nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
				    "devn-at91sam: forcing the link");
		}

		if (at91sam->cfg.duplex > 0)
			reg |= BMCR_FULL_DUPLEX;
		if (at91sam->cfg.media_rate == 100*1000)
			reg |= BMCR_SPEED_100;

		at91sam_mii_write(at91sam, at91sam->cfg.phy_addr, MDI_BMCR, reg);

		if (reg & BMCR_AN_ENABLE)
			MDI_EnableMonitor(at91sam->mdi, 1);
	} else {
		/* Not forcing the link */
		at91sam->cfg.flags |= NIC_FLAG_LINK_DOWN;
		MDI_AutoNegotiate(at91sam->mdi, at91sam->cfg.phy_addr, NoWait);
		status = MDI_EnableMonitor(at91sam->mdi, 1);

		if (status != MDI_SUCCESS)
			nic_slogf(_SLOGC_NETWORK, _SLOG_INFO,
			    "devn-at91sam: MDI_EnableMonitor returned %x", status);
	}

	return 0;
}
