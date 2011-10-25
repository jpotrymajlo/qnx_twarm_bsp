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


#include	<at91sam.h>

#define	AT91_MMC_TMOUT		10000

static	uint32_t	int_status, int_status2;

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

const   struct sigevent *at91sam_int (void *area, int id)

{
at91sam_dev_t	*ext = (at91sam_dev_t *) area;
void			*regs = (void *) ext->mmc_base;
uint32_t		status;
int				done = 0;

	status = MCI_IN32 (MCI_SR);
int_status = status;

	status &= MCI_IN32 (MCI_IMR);
int_status2 = status;
	if (status & 0xffff0000)
		done = 1;

	if (!done) {
		if (status & ENDRX) {
			MCI_OUT32 (MCI_IER, RXBUFF);
			MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);
			}

		if (status & RXBUFF) {
			MCI_OUT32 (MCI_IER, CMDRDY);
			}

		if (status & TXBUFE) {
			MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);
			MCI_OUT32 (MCI_IER, NOTBUSY);
			}

		if (status & NOTBUSY) {
			MCI_OUT32 (MCI_IER, CMDRDY);
			}

		if (status & CMDRDY)
			done = 1;
		}

	MCI_OUT32 (MCI_IDR, status);

	/*
	 * Disable all the interrupts
	 */
	if (done) {
		MCI_OUT32 (MCI_IDR, 0xffffffff);
			return (&ext->int_event);
		}

	return (NULL);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static	int	at91sam_wait_intr (at91sam_dev_t *ext)

{
struct	_pulse	pulse;
iov_t			iov;
int				rcvid;

	SETIOV (&iov, &pulse, sizeof (pulse));
	if ((rcvid = MsgReceivev (ext->chid, &iov, 1, NULL)) == -1) {
		if (errno == ESRCH) {
			ext->int_expected = 0;
			return (-1);
			}
		}
	ext->int_expected = 0;
	return (0);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static	int	at91sam_sendcmd (at91sam_dev_t *ext, uint32_t command, uint32_t argument, uint16_t status)

{
void		*regs = (void *) ext->mmc_base;
uint32_t	stat;

	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "%s: command %x - argument %x - status %x", __FUNCTION__, command, argument, status);
	MCI_OUT32 (MCI_ARGR, argument);
	if (command & 0xc0)
		MCI_OUT32 (MCI_CMDR, command | MAXLAT);
	else
		MCI_OUT32 (MCI_CMDR, command);

	if (status != 0xffff) {
		ext->int_expected = 1;
		MCI_OUT32 (MCI_IER, 0xffff0000 | status);
		if (at91sam_wait_intr (ext) == -1)
			perror ("InterruptWait");

		stat = MCI_IN32 (MCI_SR);

		if (stat & RTOE) {	//response time out
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "send command failed: command = %x, argument = %x status = %x", command, argument, stat);
			MCI_OUT32 (MCI_IDR, 0xffffffff);
			return (-1);
			}
		}
	return (0);
}

#if	0
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int at91sam_sd_version (at91sam_dev_t *ext)

{
uint32_t	status;
void		*regs = (void *) ext->mmc_base;

	if (at91sam_sendcmd (ext, (MMC_IF_COND | RSPTYPE_48 | OPDCMD), 0x1AA, CMDRDY))
		return (1);
	status = MCI_IN32 (MCI_RSPR3);

	if ((status & 0xFFFF) == 0x1AA)
		return (2);

	return (-1);
}
#endif

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int at91sam_power2 (at91sam_dev_t *ext)

{
int		i, ocr, rsp;
void	*regs = (void *) ext->mmc_base;

	for (i = 0; i < AT91_MMC_TMOUT; i++) {
		delay(1);

		/* Format and send cmd: Volt. window is usually 0x00300000 (3.4-3.2v)*/
		if (i > 0)
			ocr = 0x00ff8000;
		else
			ocr = 0;

		if (ext->version == 2)
			ocr |= 1 << 30;

		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SEND_OP_COND, ocr, CMDRDY))
			return (-1);

		rsp = MCI_IN32 (MCI_RSPR3);

		/* High capacity card ? */
		if (rsp & 0x40000000)
			ext->hc = 1;

		/* Power up ? */
		if (rsp & 0x80000000)
			return (0);
		}

	return (-1);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int at91sam_power (at91sam_dev_t *ext)

{
int		i, ocr, rsp;
void	*regs = (void *) ext->mmc_base;

	for (i = 0; i < AT91_MMC_TMOUT; i++) {
		delay(1);

		/* Send CMD55 */
		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_APP_CMD, 0, CMDRDY))
		{
			printf("kicha 1.1\r\n");
			return (-1);
		}

		/* Format and send cmd: Volt. window is usually 0x00300000 (3.4-3.2v)*/
		if (i > 0)
			ocr = 0x00ff8000;
		else
			ocr = 0;

		if (ext->version == 2)
			ocr |= 1 << 30;

		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | SD_SEND_OP_COND, ocr, CMDRDY))
		{
			printf("kicha 1.2\r\n");
			return (-1);
		}

		rsp = MCI_IN32 (MCI_RSPR3);

		/* High capacity card ? */
		if (rsp & 0x40000000)
			ext->hc = 1;

		/* Power up ? */
		if (rsp & 0x80000000) {
			slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "rsp = %x", rsp);
			return 0;
			}
		}
	printf("kicha 1.4 0x%x\r\n", rsp);

	return (-1);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int at91sam_init_mci (at91sam_dev_t *ext)

{
void		*regs = (void *) ext->mmc_base;
uintptr_t	base;
int			i;

    printf("+++ at91sam_init_mci\r\n");
	if (ext->port_base == MCI_BASE) {
		printf("mapujemy pio\r\n");
		base = mmap_device_io (0x100, 0xfffff400);
		out32 (base + 0x04, 0x18);		/* Disable SPI */
		out32 (base + 0x70, 0x27);		/* Set Periph. A */
		out32 (base + 0x64, 0x1fff);	/* Enable pull-up */
		munmap_device_io (base, 0x100);
		delay (10);
		}

	printf("Status Register %x", MCI_IN32 (MCI_SR));

	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "Status Register %x", MCI_IN32 (MCI_SR));
	MCI_OUT32 (MCI_CR, MCIDIS | SWRST);
	delay (100);
	MCI_OUT32 (MCI_CR, MCIEN | PWSDIS);
	MCI_OUT32 (MCI_IDR, 0xffffffff);
	/* Set mode and data timeout registers */
	MCI_OUT32 (MCI_MR, ((512 << 16)  | 74 | (3 << 8) | PDCMODE));
//	MCI_OUT32 (MCI_MR, ((512 << 16)  | 74 | (3 << 8) | PDCMODE | WRPROOF | RDPROOF));
	ext->speed = 0;
	MCI_OUT32 (MCI_DTOR, 0x0f | (7 << 4));
	MCI_OUT32 (MCI_SDCR, SDCBUS);

	/* Put the card in idle state */
	at91sam_sendcmd (ext, SPCMD_INIT | OPDCMD | MMC_GO_IDLE_STATE, 0, CMDRDY);

	if (at91sam_power (ext)) {
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "SD: Power up failed");
		if (at91sam_power2 (ext)) {
			slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "MMC: Power up failed");
			return (-1);
			}
		else
			slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "Good");
		}

	slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "SD: SD card version : %d", ext->version);
	if (ext->hc == 1)
		slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "SD: High capacity card detected.");

	/* Ask all cards to send their CIDs */
	if (at91sam_sendcmd (ext, RSPTYPE_136 | OPDCMD | MMC_ALL_SEND_CID, 0, CMDRDY)) {
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "send cid failed");
		return (-1);
		}
	else
		slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "send cid good");

	/* Read Relative Address from the card and store it in variable rca */
	if (at91sam_sendcmd (ext, RSPTYPE_48 | MMC_SET_RELATIVE_ADDR, 0, CMDRDY))
		return (-1);
	ext->mmc_rca = (MCI_IN32 (MCI_RSPR3) >> 16);

	/* Select the Card which responded */
	if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SEL_DES_CARD, ext->mmc_rca << 16, CMDRDY))
		return (-1);

	if (ext->mmc_rca > 0) {
		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SEND_STATUS, ext->mmc_rca << 16, CMDRDY))
			return (-1);
		}

	/* Set bus width as 4 */
	for (i = 0; i < AT91_MMC_TMOUT; i++) {
		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_APP_CMD, ext->mmc_rca << 16, CMDRDY))
			return (-1);
		if (!at91sam_sendcmd (ext, RSPTYPE_48 | SET_BUS_WIDTH, 2, CMDRDY))
			break;
		delay (1);
		}

	if (i >= AT91_MMC_TMOUT)
		return (-1);

	/* Set block length */
	if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SET_BLOCKLEN, MMC_DFLT_BLKSIZE, CMDRDY))
		return (-1);

	return (MMC_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

int		at91sam_init (SIM_HBA *hba, at91sam_dev_t *ext)

{
CONFIG_INFO *cfg = (CONFIG_INFO *)&hba->cfg;
uint32_t	pbase, irq;

   printf("+++ at91sam_init\r\n");
	if ((ext->chid = ChannelCreate (_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK)) == -1) {
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "SD: Unable to create channel");
		printf("SD: Unable to create channel");
		return (MMC_ALLOC_FAILED);
		}

	if ((ext->coid = ConnectAttach (0, 0, ext->chid, _NTO_SIDE_CHANNEL, 0 )) == -1) {
		printf("SD: Unable to attach channel");
		goto fail1;
		}

	ext->port_base = pbase = cfg->NumIOPorts ? cfg->IOPort_Base[0] : MCI_BASE;
	ext->mmc_irq = irq = cfg->NumIRQs ? cfg->IRQRegisters[0] : 10;
	printf("pbase %x - irq %x\r\n", pbase, irq);

	if (ext->verbose)
		slogf (_SLOGC_SIM_MMC, _SLOG_INFO, "pbase %x - irq %x", pbase, irq);
	if ((ext->mmc_base = mmap_device_io (MCI_BASE_SIZE, pbase)) == (uintptr_t)MAP_FAILED) {
		slogf (_SLOGC_SIM_MMC, _SLOG_ERROR, "SD: mmap_device_io failed");
		printf("SD: mmap_device_io failed\r\n");
		goto fail2;
		}
    printf("*** kicha 1\r\n");
	SIGEV_PULSE_INIT (&ext->int_event, ext->coid, getprio(0), 3, 0);
    printf("*** kicha 2\r\n");

	ext->int_expected = 0;
    printf("*** kicha 3\r\n");

	if ((ext->int_id = InterruptAttach (ext->mmc_irq, at91sam_int, (void *)ext, sizeof(*ext), _NTO_INTR_FLAGS_PROCESS)) == -1) {
		printf("SD: InterruptAttach failed\r\n");
		goto fail3;
		}
    printf("*** kicha 4\r\n");

	if (at91sam_init_mci (ext)) {
		printf("SD: card init failed\r\n");
		goto fail4;
		}
	   printf("--- at91sam_init\r\n");

	return (MMC_SUCCESS);

fail4:
	InterruptDetach (ext->int_id);
fail3:
	munmap_device_io (ext->mmc_base, MCI_BASE_SIZE);
fail2:
	ConnectDetach (ext->coid);
fail1:
	ChannelDestroy (ext->chid);

	return (MMC_ALLOC_FAILED);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int mmc_ready (at91sam_dev_t *ext)

{
int		i;
void	*regs = (void *) ext->mmc_base;

	/*
	 * Make sure the card is ready for data
	 */
	for (i = 0; i < AT91_MMC_TMOUT; i++) {
//		if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SEND_STATUS, ext->mmc_rca << 16, CMDRDY) == 0)
		if (at91sam_sendcmd (ext, RSPTYPE_48 | MMC_SEND_STATUS, ext->mmc_rca << 16, CMDRDY) == 0)
			if (MCI_IN32 (MCI_RSPR3) & (MMC_READY_FOR_DATA))
				return (0);
		}

	return (-1);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int	mmc_read_blk (at91sam_dev_t *ext, uint32_t lba, uint8_t *data, int len)

{
uint32_t	cmdr;
int			done = len;
void 		*regs = (void *) ext->mmc_base;

	if (! ext->speed) {
		cmdr = (MCI_IN32 (MCI_MR) & ~0xff);
		cmdr |= 4;
		MCI_OUT32 (MCI_MR, cmdr);
		delay (10);
		ext->speed = 1;
		if (ext->verbose)
			slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "%s: setting speed %x", __FUNCTION__, cmdr);
		}

	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "read blk = %d, len = %x", lba, len);
	if (!ext->hc)
		lba *= MMC_DFLT_BLKSIZE;

	while (1) {
		if (MCI_IN32 (MCI_SR) & NOTBUSY)
			break;
		}
	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "TCR %x - Stat %x", MCI_IN32 (MCI_TCR), MCI_IN32 (MCI_SR));
//	cmdr = (RSPTYPE_48 | TRDIR | TRCMD_START | OPDCMD | MAXLAT);
	cmdr = (RSPTYPE_48 | TRDIR | TRCMD_START | MAXLAT);

	if (len == MMC_DFLT_BLKSIZE)
		cmdr |= MMC_READ_SINGLE_BLOCK;
	else
		cmdr |= (MMC_READ_MULTIPLE_BLOCK | TRTYP_MB);

	/*
	 * Disable PDC
	 */
	MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);

	/*
	 * Setup receive DMA
	 */
	MCI_OUT32 (MCI_RPR, (uint32_t)data);
	MCI_OUT32 (MCI_RCR, len / 4);
	MCI_OUT32 (MCI_RNCR, 0);

	at91sam_sendcmd (ext, cmdr, lba, -1);
	MCI_OUT32 (MCI_PTCR, MCI_RXTEN);	/* Enable receive DMA */
	ext->int_expected = 1;

	MCI_OUT32 (MCI_IDR, 0xFFFFFFFF);
	MCI_OUT32 (MCI_IER, ENDRX);
	if (at91sam_wait_intr (ext) == -1)
		perror ("InterruptWait");

	if (MCI_IN32 (MCI_SR) & 0xFFFF0000)
		done = 0;

	/*
	 * Stop DMA
	 */
	MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);

	/*
	 * Send STOP_TRANSMISSION command
	 */
	if (len != MMC_DFLT_BLKSIZE) {
		MCI_OUT32 (MCI_RPR, 0);
		MCI_OUT32 (MCI_RCR, 0);
		MCI_OUT32 (MCI_RNCR, 0);
		MCI_OUT32 (MCI_RNPR, 0);
		MCI_OUT32 (MCI_TPR, 0);
		MCI_OUT32 (MCI_TCR, 0);
		MCI_OUT32 (MCI_TNCR, 0);
		MCI_OUT32 (MCI_TNPR, 0);
//		if (at91sam_sendcmd (ext, TRDIR | TRCMD_STOP | OPDCMD | RSPTYPE_48 | MMC_STOP_TRANSMISSION, 0, CMDRDY))
		if (at91sam_sendcmd (ext, TRDIR | TRCMD_STOP | RSPTYPE_48 | MMC_STOP_TRANSMISSION, 0, CMDRDY))
			done = 0;

		if (MCI_IN32 (MCI_SR) & RXRDY) {

			MCI_OUT32 (MCI_CR, SWRST);
			MCI_IN32 (MCI_SR);
			MCI_OUT32 (MCI_CR, PWSDIS | MCIEN);
			MCI_OUT32 (MCI_DTOR, 0x7F);
			MCI_OUT32 (MCI_MR, (512 << 16) | PDCMODE | 0x320);
			MCI_OUT32 (MCI_SDCR, SDCBUS);
			}
		}


	return (done);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int	mmc_write_blk (at91sam_dev_t *ext, uint32_t lba, uint8_t *data, int len)

{
uint32_t	cmdr, mr;
int			done = len;
void		*regs = (void *) ext->mmc_base;

	if (! ext->speed) {
		mr = (MCI_IN32 (MCI_MR) & ~0xff);
		mr |= 4;
		MCI_OUT32 (MCI_MR, mr);
		delay (10);
		ext->speed = 1;
		if (ext->verbose)
			slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "%s: setting speed %x", __FUNCTION__, mr);
		}

	if (ext->verbose > 2) {
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "write block, blk = %x , len = %x", (uint32_t)lba, len);
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "write status = %x", MCI_IN32 (MCI_SR));
		}
	if (!ext->hc)
		lba *= MMC_DFLT_BLKSIZE;

//	cmdr = (RSPTYPE_48 | TRCMD_START | OPDCMD | MAXLAT);
	cmdr = (RSPTYPE_48 | TRCMD_START | MAXLAT);

	if (len == MMC_DFLT_BLKSIZE)
		cmdr |= MMC_WRITE_BLOCK;
	else
		cmdr |= (MMC_WRITE_MULTIPLE_BLOCK | TRTYP_MB);

	while (1) {
		if (MCI_IN32 (MCI_SR) & NOTBUSY)
			break;
		}
	MCI_OUT32 (MCI_IDR, 0xFFFFFFFF);
	mr = MCI_IN32 (MCI_MR) & 0x7FFF;
	MCI_OUT32 (MCI_MR, mr | (512 << 16) | PDCMODE);

	/*
	 * Disable PDC
	 */
	MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);

	/*
	 * Setup transmit DMA
	 */
	MCI_OUT32 (MCI_TPR, (uint32_t)data);
	MCI_OUT32 (MCI_TCR, len / 4);

	at91sam_sendcmd (ext, cmdr, lba, -1);	/* no status check */
	MCI_OUT32 (MCI_IER, TXBUFE | 0xffff0000);
	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "Int Mask %x", MCI_IN32 (MCI_IMR));
	MCI_OUT32 (MCI_PTCR, MCI_TXTEN);		/* Enable transmit DMA */

	ext->int_expected = 1;
	if (at91sam_wait_intr (ext) == -1)
		perror ("InterruptWait");
	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "Write stat %x - stat2 %x", int_status, int_status2);
	cmdr = MCI_IN32 (MCI_SR);
	if (ext->verbose > 2)
		slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "Write status %x - tcr %x", cmdr, MCI_IN32 (MCI_TCR));
	if (cmdr & 0xFFFF0000)
		done = 0;
	else {
		if (!(cmdr & TXBUFE)) {
			MCI_OUT32 (MCI_IER, TXBUFE | 0xffff0000);
			ext->int_expected = 1;
			if (at91sam_wait_intr (ext) == -1)
				perror ("InterruptWait");
			cmdr = MCI_IN32 (MCI_SR);
			if (ext->verbose > 2)
				slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "Write status1 %x - tcr %x", cmdr, MCI_IN32 (MCI_TCR));
			if (!(cmdr & NOTBUSY)) {
				MCI_OUT32 (MCI_IDR, TXBUFE);
				MCI_OUT32 (MCI_IER, NOTBUSY);
				ext->int_expected = 1;
				if (at91sam_wait_intr (ext) == -1)
					perror ("InterruptWait");
				}
			}
		}
	/*
	 * Stop DMA
	 */
	MCI_OUT32 (MCI_PTCR, MCI_RXTDIS | MCI_TXTDIS);

	/*
	 * Send STOP_TRANSMISSION command
	 */
	if (len != MMC_DFLT_BLKSIZE) {
		printf ("stop write \n");
//		if (at91sam_sendcmd (ext, TRCMD_STOP | OPDCMD | RSPTYPE_48 | MMC_STOP_TRANSMISSION, 0, CMDRDY))
		if (at91sam_sendcmd (ext, TRCMD_STOP | RSPTYPE_48 | MMC_STOP_TRANSMISSION, 0, CMDRDY))
			done = 0;
		printf ("stop write status = %x\n", MCI_IN32 (MCI_SR));
		if (0) { //in32(base + MCI_SR) & (1 << 1)) {

		MCI_OUT32 (MCI_CR, SWRST);
		delay (1);
		MCI_OUT32 (MCI_CR, PWSDIS | MCIEN);
		MCI_OUT32 (MCI_DTOR, 0x7F);
		MCI_OUT32 (MCI_MR, (512 << 16) | PDCMODE | 0x320);
		MCI_OUT32 (MCI_SDCR, SDCBUS);
		}
	}

	return (done);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

static int at91sam_send_cid_csd (at91sam_dev_t *ext, char *buf, int command)

{
uint32_t	*pbuf = (unsigned *)buf;
uint32_t	mr;
void		*regs = (void *) ext->mmc_base;

	if (ext->speed) {
		mr = (MCI_IN32 (MCI_MR) & ~0xff);
		mr |= 74;
		MCI_OUT32 (MCI_MR, mr);
		delay (10);
		ext->speed = 0;
		if (ext->verbose)
			slogf (_SLOGC_SIM_MMC, _SLOG_DEBUG1, "%s: setting speed %x", __FUNCTION__, mr);
		}

	/* Put the card to standby state */
	at91sam_sendcmd (ext, RSPTYPE_48 | MMC_SEL_DES_CARD, 0, -1);	//1 << 0);

	delay (5);

	/* Ask card to send CID / CSD */
	if (at91sam_sendcmd (ext, RSPTYPE_136 | OPDCMD | command, ext->mmc_rca << 16, CMDRDY))
		return (-1);

	pbuf[3] = MCI_IN32 (MCI_RSPR0);
	pbuf[2] = MCI_IN32 (MCI_RSPR1);
	pbuf[1] = MCI_IN32 (MCI_RSPR2);
	pbuf[0] = MCI_IN32 (MCI_RSPR3);

	/* Put the card back to data transfer mode */
	if (at91sam_sendcmd (ext, RSPTYPE_48 | OPDCMD | MMC_SEL_DES_CARD, ext->mmc_rca << 16, CMDRDY)) {
		delay (10);
		return (-1);
		}

	delay (10);

	return 0;
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

int		mmc_io (void *hdl, mmc_io_t *mmc)

{
uint32_t		len, status = MMC_SUCCESS;
at91sam_dev_t	*ext = (at91sam_dev_t *) hdl;

	if (hdl == NULL)
		return (MMC_INVALID_HANDLE);

	switch (mmc->mmc_cmd) {
		case	MMC_GO_IDLE_STATE:
			at91sam_sendcmd (ext, SPCMD_INIT | OPDCMD | MMC_GO_IDLE_STATE, 0, CMDRDY);
			break;
		case	MMC_SEND_CSD:
		case	MMC_SEND_CID:
			if (at91sam_send_cid_csd (ext, mmc->mmc_data, mmc->mmc_cmd))
				status = MMC_ERROR;
			break;
		case	MMC_READ_SINGLE_BLOCK:
		case	MMC_READ_MULTIPLE_BLOCK:
			len = mmc->mmc_data_len;
			{
			uint32_t	lba = mmc->mmc_arg;
			uint32_t	done;
			uint8_t		*buf = (uint8_t *)mmc->mmc_data;

			while (len) {
				done = 512;

				if (mmc_read_blk (ext, lba, buf, done) != done) {
					status = MMC_READ_ERROR;
					break;
					}
				lba += done / 512, buf += done, len -= done;
				}
			}
			break;
		case	MMC_WRITE_BLOCK:
		case	MMC_WRITE_MULTIPLE_BLOCK:
			len = mmc->mmc_data_len;
			if ((len == 0) || ((len & (MMC_DFLT_BLKSIZE - 1)) != 0)) {
				status = MMC_WRITE_ERROR;
				break;
				}
			/*
			 * Some cards don't like large buffer write,
			 * limit the write buffer to 4K
			 */
			{
			uint32_t	lba = mmc->mmc_arg;
			uint32_t	done;
			uint8_t		*buf = (uint8_t *)mmc->mmc_data;

			while (mmc_ready(ext) != 0) ;

			while (len) {
				done = 512;//(len >= 4096) ? 4096 : len;

				if (mmc_write_blk (ext, lba, buf, done) != done) {
					status = MMC_WRITE_ERROR;
					break;
					}
				lba += done / 512, buf += done, len -= done;
				while (mmc_ready(ext) != 0) ;
				}
			}
			break;
		default:
			status = MMC_COMMAND_FAILURE;
			break;
		}
	mmc->mmc_status = status;

	return (status ? MMC_FAILURE : MMC_SUCCESS);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

void	*mmc_attach (void *hdl, char *opts)

{
SIM_HBA			*hba = (SIM_HBA *)hdl;
at91sam_dev_t	*ext;
int				status;
    printf("+++ mmc_attach\r\n");
	if ((ext= calloc (1, sizeof (at91sam_dev_t))) == NULL)
	{
		printf("*** zwracamy nula\r\n");
		return (NULL);
	}

	ext->verbose = hba->verbosity;
	if ((status = at91sam_init (hba, ext)) != MMC_SUCCESS) {
		printf("*** pudlo");
		free (ext);
		return (NULL);
		}

	ext->hba = hdl;
    printf("--- mmc_attach\r\n");
	return (ext);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

int		mmc_detach (void *hdl)

{
at91sam_dev_t	*ext = (at91sam_dev_t *) hdl;
void			*regs = (void *) ext->mmc_base;

	MCI_OUT32 (MCI_MR, 0);
	MCI_OUT32 (MCI_CMDR, 0);
	MCI_OUT32 (MCI_CR, (MCIDIS | SWRST));

	InterruptDetach (ext->int_id);
	ConnectDetach (ext->coid);
	ChannelDestroy (ext->chid);

	munmap_device_io (ext->mmc_base, MCI_BASE_SIZE);
	free (ext);
	return (EOK);
}
