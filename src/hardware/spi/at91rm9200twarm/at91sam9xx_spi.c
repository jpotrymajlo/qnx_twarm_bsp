/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, 2009 QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */




#include "at91sam9xx_spi.h"

#define FIXED_PERIPHERAL
#define ENABLE_PDC

static char *at91sam9xx_opts[] = {
	"base",			/* Base address for this SPI controller */
	"irq",			/* IRQ for this SPI intereface */
	"clock",		/* SPI clock */
	"loopback", 		/*loopback interface for test purposes*/
	NULL
};

spi_funcs_t spi_drv_entry = {
	sizeof(spi_funcs_t),
	at91sam9xx_init,		/* init() */
	at91sam9xx_dinit,		/* fini() */
	at91sam9xx_drvinfo,	/* drvinfo() */
	at91sam9xx_devinfo,	/* devinfo() */
	at91sam9xx_setcfg,	/* setcfg() */
	at91sam9xx_xfer,		/* xfer() */
#if defined(ENABLE_PDC)
	at91sam9xx_dmaxfer	/* dma_xfer() */
#else
	NULL	/* dma_xfer() */
#endif
};

/*
 * Note:
 * The devices listed are just examples, users should change
 * this according to their own hardware spec.
 */
static spi_devinfo_t devlist[4] = {
	{
		0x00,				// Device ID, for SS0
		"SPI-DATAFLASH",    // Description
		{ 
			8 | SPI_MODE_CKPHASE_HALF, //Data length 8bit, MSB
			1000000			// Clock rate 1M
		},
	},
	{
		0x01,				// Device ID, for SS1
		"SPI-DEV1",		// Description
		{ 
			16,		        // data length 16bit, MSB
			5000000			// Clock rate 5M
		},
	},
	{
		0x02,				// Device ID, for SS2
		"SPI-DEV2",		// Description
		{ 
			16,		        // data length 16bit, MSB
			5000000			// Clock rate 5M
		},
	},
	{
		0x03,				// Device ID, for SS3
		"SPI-DEV3",		// Description
		{ 
			16,		        // data length 16bit, MSB
			5000000			// Clock rate 5M
		},
	},
};

static uint32_t devctrl[4];

static int at91sam9xx_options(at91sam9xx_spi_t *dev, char *optstring)
{
	int		opt, rc = 0, err = EOK;
	char	*options, *freeptr, *c, *value;

	if (optstring == NULL)
		return 0;

	freeptr = options = strdup(optstring);

	while (options && *options != '\0') {
		c = options;
		if ((opt = getsubopt(&options, at91sam9xx_opts, &value)) == -1)
			goto error;

		switch (opt) {
			case 0:
				dev->pbase = strtoul(value, 0, 0); 
				continue;
			case 1:
				dev->irq   = strtoul(value, 0, 0);
				continue;
			case 2:
				dev->clock = strtoul(value, 0, 0);
				continue;
			case 3:
				dev->loopback = 1;
				continue;
		}
error:
		fprintf(stderr, "spi-at91sam9xx: unknown option %s", c);
		err = EINVAL;
		rc = -1;
	}

	free(freeptr);

	return rc;
}

void *at91sam9xx_init(void *hdl, char *options)
{
	at91sam9xx_spi_t	*dev;
	uintptr_t	base;
	int			i;
    
	dev = calloc(1, sizeof(at91sam9xx_spi_t));

	if (dev == NULL)
		return NULL;

    //Set defaults
	dev->pbase = AT91_SPI_BASE;
	dev->irq   = AT91_SPI_IRQ;
	dev->clock = 100000000;			/* 100 MHz Master clock */
    dev->loopback = 0;
    
	if (at91sam9xx_options(dev, options))
		goto fail0;
    
	/*
	 * Map in SPI registers
	 */
	if ((base = mmap_device_io(AT91_SPI_SIZE, dev->pbase)) == (uintptr_t)MAP_FAILED)
		goto fail0;

	dev->vbase = base;

    /* enable interface */
	out32 (base + AT91_SPI_CONREG, SPI_CONREG_SWRST);
#if defined(FIXED_PERIPHERAL)
	out32 (base + AT91_SPI_MODEREG, SPI_MODEREG_MODFDIS | SPI_MODEREG_MSTR | SPI_MODEREG_PCS(0xf));
#else
	out32 (base + AT91_SPI_MODEREG, SPI_MODEREG_PS | SPI_MODEREG_MODFDIS | SPI_MODEREG_MSTR);
#endif
#if defined(ENABLE_PDC)
	/* Disable SPI PDC transfer */
        out32(base + PDC_PTCR, PTCR_RXTDIS | PTCR_TXTDIS);
#endif
	out32 (base + AT91_SPI_CONREG, SPI_CONREG_SPIEN);

   if (dev->loopback == 1) {
       out32(base + AT91_SPI_MODEREG, in32 (base + AT91_SPI_MODEREG) | SPI_MODEREG_LOOPBACK);
   }
   
	/*
	 * Calculate all device configuration here
	 */
	for (i = 0; i < 4; i++)
		devctrl[i] = at91sam9xx_cfg(dev, &devlist[i].cfg);
    
	/*
	 * Attach SPI interrupt
	 */
	if (at91sam9xx_attach_intr(dev))
		goto fail1;
	/* Unmask Tx & Rx intr */
#if !defined(ENABLE_PDC)
	out32 (base + AT91_SPI_INTMASKREG, SPI_INTREG_RDRF|SPI_INTREG_TRDE);
#endif    
	dev->spi.hdl = hdl;

   	return dev;

fail1:
	munmap_device_io(dev->vbase, 0x20);
fail0:
	free(dev);
	return NULL;
}

void at91sam9xx_dinit(void *hdl)
{
	at91sam9xx_spi_t	*dev = hdl;
    
	/*
	 * unmap the register, detach the interrupt
	 */
	InterruptDetach(dev->iid);
	ConnectDetach(dev->coid);
	ChannelDestroy(dev->chid);

	/*
	 * Disable SPI
	 */
    out32 (dev->vbase + AT91_SPI_CONREG, SPI_CONREG_SPIEN);
	munmap_device_io(dev->vbase, AT91_SPI_SIZE);

	free(hdl);
}

int at91sam9xx_drvinfo(void *hdl, spi_drvinfo_t *info)
{
	info->version = (SPI_VERSION_MAJOR << SPI_VERMAJOR_SHIFT) | (SPI_VERSION_MINOR << SPI_VERMINOR_SHIFT) | (SPI_REVISION << SPI_VERREV_SHIFT);
	strcpy(info->name, "AT91SAM9xx SPI");
	info->feature = 0;
	return (EOK);
}

int at91sam9xx_setcfg(void *hdl, uint16_t device, spi_cfg_t *cfg)
{
	uint32_t	control;

	if (device > 3)
		return (EINVAL);

	control = at91sam9xx_cfg(hdl, &devlist[device].cfg);
	if (control == 0)
		return (EINVAL);

	memcpy(&devlist[device].cfg, cfg, sizeof(spi_cfg_t));
	devctrl[device] = control;

	return (EOK);
}

int at91sam9xx_devinfo(void *hdl, uint32_t device, spi_devinfo_t *info)
{
	int		dev = device & SPI_DEV_ID_MASK;
    
	if ((device & SPI_DEV_DEFAULT))
        dev = 0;
    
	if (dev > 3)
		return (EINVAL);

	memcpy(info, &devlist[dev], sizeof(spi_devinfo_t));
	return (EOK);
}


void *at91sam9xx_xfer(void *hdl, uint32_t device, uint8_t *buf, int *len)
{
	at91sam9xx_spi_t	*dev = hdl;
	uintptr_t	base = dev->vbase;
	uint32_t	id;

	id = device & SPI_DEV_ID_MASK;
	if (id > 3) {
		*len = -1;
		return buf;
	}

	dev->curr_cs = id;
	dev->xlen = *len;
	dev->rlen = 0;
	dev->tlen = 0;
	dev->pbuf = buf;
	dev->dlen = ((devlist[id].cfg.mode & SPI_MODE_CHAR_LEN_MASK) + 7) >> 3;

    // Estimate transfer time in us... The calculated dtime is only used for
    // the timeout, so it doesn't have to be that accurate.  At higher clock
    // rates, a calcuated dtime of 0 would mess-up the timeout calculation, so
    // round up to 1 us
	dev->dtime = dev->dlen * 8 * 1000 * 1000 / devlist[id].cfg.clock_rate;
    if (dev->dtime == 0)
        dev->dtime = 1;

	/* Set parameters for this device */
    out32 (base + AT91_SPI_CHIPSEL0 + 4*id, devctrl[id]);

    // enable xfer complete interrupt
    out32(base + AT91_SPI_INTENREG, SPI_INTREG_RDRF);    
        
    switch (dev->dlen) {
        case 1:
            out32(base + AT91_SPI_TXDATA, buf[0] | (id << SPI_TXDATA_PCS_POS));
            dev->tlen++;
            break;
        case 2:
            out32(base + AT91_SPI_TXDATA, (*(uint16_t *)(&buf[0]))
			                               | (id << SPI_TXDATA_PCS_POS));
            dev->tlen += 2;
            break;
    }
    
    /*
     * Wait for exchange to finish
     */
          
     if (at91sam9xx_wait(dev, dev->xlen * 10)) {
        fprintf(stderr, "spi-at91sam9xx: XFER Timeout!!!");
        dev->rlen = -1;
     }
     
	*len = dev->rlen;

	return buf;
}

#if defined (ENABLE_PDC)
int at91sam9xx_dmaxfer( void *hdl, uint32_t device, spi_dma_paddr_t *paddr, int len )
{
	at91sam9xx_spi_t	*dev = hdl;
	uint32_t	id;
	uint32_t	mr;
	uintptr_t	base = dev->vbase;
	uint32_t	cmdsize, responsesize;

	/* Disable SPI PDC transfer */
        out32(base + PDC_PTCR, PTCR_RXTDIS | PTCR_TXTDIS);

	/* Select chip in MR - should be same as xfer*/
	id = device & SPI_DEV_ID_MASK;
	dev->curr_cs = id;

	/* setup count parameters for PDC */
	responsesize = len & 0x0000ffff;
	cmdsize = (len & 0xffff0000)>>16;

	dev->dlen = ((devlist[id].cfg.mode & SPI_MODE_CHAR_LEN_MASK) + 7) >> 3;

    // Estimate transfer time in us... The calculated dtime is only used for
    // the timeout, so it doesn't have to be that accurate.  At higher clock
    // rates, a calcuated dtime of 0 would mess-up the timeout calculation, so
    // round up to 1 us
	dev->dtime = dev->dlen * 8 * 1000 * 1000 / devlist[id].cfg.clock_rate;
    if (dev->dtime == 0)
        dev->dtime = 1;

	/* Set parameters for this device */
	out32 (base + AT91_SPI_CHIPSEL0 + 4*id, devctrl[id]);
	mr = in32(base + AT91_SPI_MODEREG);
	mr |= SPI_MODEREG_PCS(0xf);
	mr &= ~SPI_MODEREG_PCS(0x1 << id);
	out32 (base + AT91_SPI_MODEREG,mr);

	/* setup buffer for PDC */
	out32(base + PDC_RPR, (uint32_t)paddr->wpaddr);
	out32(base + PDC_RCR, cmdsize);
	out32(base + PDC_TPR, (uint32_t)paddr->wpaddr);
	out32(base + PDC_TCR, cmdsize);
	out32(base + PDC_RNPR, (uint32_t)paddr->rpaddr);
	out32(base + PDC_RNCR, responsesize);
	out32(base + PDC_TNPR, (uint32_t)paddr->rpaddr);
	out32(base + PDC_TNCR, responsesize);
	/* Enable TX and RX Interrupts */
        out32(base + PDC_PTCR, PTCR_RXTEN | PTCR_TXTEN);
	/* Enable RX Buffer complete interrupt */
        out32(base + AT91_SPI_INTENREG, 0x1<<6 ); //AT91C_SPI_RXBUFF

    /*
     * Wait for exchange to finish
     */
          
     if (at91sam9xx_wait(dev, dev->xlen * 10)) {
        fprintf(stderr, "spi-at91sam9xx: XFER Timeout!!!");
        dev->rlen = -1;
     }
	 /* Disable SPI PDC transfer */
	out32 (base + PDC_PTCR, PTCR_RXTDIS | PTCR_TXTDIS );

#if SPI_PDC_DEBUG 
	/* DMA complete should see all zero in RCR, TCR, RNCR, TNCR */
	fprintf(stderr, "PDC_RCR after dma %d\n", in32(base + PDC_RCR));
	fprintf(stderr, "PDC_TCR after dma %d\n", in32(base + PDC_TCR));
	fprintf(stderr, "PDC_RNCR after dma %d\n", in32(base + PDC_RNCR));
	fprintf(stderr, "PDC_TNCR after dma %d\n", in32(base + PDC_TNCR));
	fprintf(stderr, "at91sam9xx_dmaxfer: %d\n", len);
#endif
	
	return 0;
}
#endif
