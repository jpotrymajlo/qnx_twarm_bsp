/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems.
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




/*
 * AT91RM9200 processor
 */

#ifndef	__ARM_AT91RM9200_H_INCLUDED
#define	__ARM_AT91RM9200_H_INCLUDED

/*---------------------------------------
          Internal Memory Mapping
  ---------------------------------------*/
#define	AT91RM9200_BOOT_BASE	0x00000000
#define	AT91RM9200_ROM_BASE     0x00100000
#define	AT91RM9200_SRAM_BASE	0x00200000
#define	AT91RM9200_USBH_BASE	0x00300000


/*
 * External Bus Interface 0 & 1 Mapping
 */
#define AT91RM9200_EBI0_CS0_BASE               0x10000000
#define AT91RM9200_EBI0_CS1_BASE               0x20000000  /* SDRAMC    */
#define AT91RM9200_EBI0_CS2_BASE               0x30000000
#define AT91RM9200_EBI0_CS3_BASE               0x40000000  /* NANDFlash */
#define AT91RM9200_EBI0_CS4_BASE               0x50000000  /* Compact Flash Slot 0 */
#define AT91RM9200_EBI0_CS5_BASE               0x60000000  /* Compact Flash Slot 1 */
#define AT91RM9200_EBI1_CS0_BASE               0x70000000
#define AT91RM9200_EBI1_CS1_BASE               0x80000000  /* SDRAMC    */
#define AT91RM9200_EBI1_CS2_BASE               0x90000000  /* NANDFlash */

/*---------------------------------------
            User Peripheral
  ---------------------------------------*/

/*
 * Timer Counter (TC)
 */
#define	AT91RM9200_TC0_BASE		0xFFFA0000
#define	AT91RM9200_TC3_BASE		0xFFFA4000
#define	AT91RM9200_TC_SIZE		0x100

#define	AT91RM9200_TC_CCR		0x00
#define	AT91RM9200_TC_CMR		0x04
#define	AT91RM9200_TC_CV		0x10
#define	AT91RM9200_TC_RA		0x14
#define	AT91RM9200_TC_RB		0x18
#define	AT91RM9200_TC_RC		0x1C
#define	AT91RM9200_TC_SR		0x20
#define	AT91RM9200_TC_IER		0x24
#define	AT91RM9200_TC_IDR		0x28
#define	AT91RM9200_TC_IMR		0x2C

#define	AT91RM9200_TC_BCR		0xC0
#define	AT91RM9200_TC_BMR		0xC4


#define	AT91RM9200_TC_CCR_CLKEN		(1 << 0)
#define	AT91RM9200_TC_CCR_CLKDIS	(1 << 1)
#define	AT91RM9200_TC_CCR_SWTRG		(1 << 2)

#define	AT91RM9200_TC_CMR_CPCTRG	(1 << 14)

/*
 * USB Device Port (UDP)
 */
#define	AT91RM9200_UDP_BASE		0xFFFB0000

/*
 * Multimedia Card Interface (MCI)
 */
#define	AT91RM9200_MCI_BASE		0xFFFB4000

/*
 * Two Wire Interface (TWI)
 */
#define	AT91RM9200_TWI_BASE		0xFFFB8000

/*
 * Ethernet MAC (EMAC)
 */
#define	AT91RM9200_EMAC_BASE	0xFFFBC000
#define	AT91RM9200_EMAC_SIZE	0xC0

/*
 * Universal Synchronous Asynchronous Receiver Transceiver (USART)
 */
#define	AT91RM9200_USART0_BASE	0xFFFC0000
#define	AT91RM9200_USART1_BASE	0xFFFC4000
#define	AT91RM9200_USART2_BASE	0xFFFC8000
#define	AT91RM9200_USART3_BASE	0xFFFCC000

#define	AT91USART_SIZE			0x100

/*
 * --------------------------------------------------------------------------
 * Atmel USAR0 Registers
 * --------------------------------------------------------------------------
 */
#define	AT91USART_CR			0x00	/* Control Register */
#define	AT91USART_MR			0x04	/* Mode Register */
#define	AT91USART_IER			0x08	/* Interrupt Enable Register */
#define	AT91USART_IDR			0x0C	/* Interrupt Disable Register */
#define	AT91USART_IMR			0x10	/* Interrupt Mask Register */
#define	AT91USART_CSR			0x14	/* Channel Status Register */
#define	AT91USART_RHR			0x18	/* Receiver Holding Register */
#define	AT91USART_THR			0x1C	/* Transmitter Holding Register */
#define	AT91USART_BRGR			0x20	/* Baud Rate Generator Register */
#define	AT91USART_RTOR			0x24	/* Receiver Time-out Register */
#define	AT91USART_TTGR			0x28	/* Transmitter Timeguard Register */
#define	AT91USART_FIDI			0x40	/* FIDI Ratio Register */
#define	AT91USART_NER			0x44	/* Number of Errors Register */
#define	AT91USART_IF			0x4C	/* IrDA Filter Register */


/*
 * CR register bits
 */
#define	AT91USART_CR_RSTRX		(1 << 2)	/* Reset Receiver */
#define	AT91USART_CR_RSTTX		(1 << 3)	/* Reset Transmitter */
#define	AT91USART_CR_RXEN		(1 << 4)	/* Receiver Enable */
#define	AT91USART_CR_RXDIS		(1 << 5)	/* Receiver Disable */
#define	AT91USART_CR_TXEN		(1 << 6)	/* Transmitter Enable */
#define	AT91USART_CR_TXDIS		(1 << 7)	/* Transmitter Disable */
#define	AT91USART_CR_RSTSTA		(1 << 8)	/* Reset Status Bits */
#define	AT91USART_CR_STTBRK		(1 << 9)	/* Start Break */
#define	AT91USART_CR_STPBRK		(1 << 10)	/* Stop Break */
#define	AT91USART_CR_STTTO		(1 << 11)	/* Start Time-out */
#define	AT91USART_CR_SENDA		(1 << 12)	/* Send Address */
#define	AT91USART_CR_RSTIT		(1 << 13)	/* Reset Iterations */
#define	AT91USART_CR_RSTNACK	(1 << 14)	/* Reset Non Acknowledge */
#define	AT91USART_CR_RETTO		(1 << 15)	/* Rearm Time-out */
#define	AT91USART_CR_DTREN		(1 << 16)	/* DTE Enable */
#define	AT91USART_CR_DTRDIS		(1 << 17)	/* DTE Disable */
#define	AT91USART_CR_RTSEN		(1 << 18)	/* RTS Enable */
#define	AT91USART_CR_RTSDIS		(1 << 19)	/* RTS Disable */

/*
 * Interrupt Enable/Disable/Mask/Status register
 */
#define	AT91USART_INT_RXRDY		(1 << 0)	/* Receiver Ready */
#define	AT91USART_INT_TXRDY		(1 << 1)	/* Transmitter Ready */
#define	AT91USART_INT_RXBRK		(1 << 2)	/* Break Received/End of Break */
#define	AT91USART_INT_ENDRX		(1 << 3)	/* End of Receive Transfer */
#define	AT91USART_INT_ENDTX		(1 << 4)	/* End of Transmitter Transfer */
#define	AT91USART_INT_OVRE		(1 << 5)	/* Overrun Error */
#define	AT91USART_INT_FRAME		(1 << 6)	/* Framing Error */
#define	AT91USART_INT_PARE		(1 << 7)	/* Parity Error */
#define	AT91USART_INT_TIMEOUT	(1 << 8)	/* Receive Timeout */
#define	AT91USART_INT_TXEMPTY	(1 << 9)	/* Transmitter Empty */
#define	AT91USART_INT_ITERATION	(1 << 10)	/* Max Number of Repetitions reached */
#define	AT91USART_INT_TXBUFE	(1 << 11)	/* Transmission Buffer Empty */
#define	AT91USART_INT_RXBUFF	(1 << 12)	/* Receive Buffer Full */
#define	AT91USART_INT_NACK		(1 << 13)	/* Receive Buffer Full */
#define	AT91USART_INT_RIIC		(1 << 16)	/* Ring Indicator Input Change */
#define	AT91USART_INT_DSRIC		(1 << 17)	/* Data Set Ready Input Change */
#define	AT91USART_INT_DCDIC		(1 << 18)	/* Data Carrier Detect Input Change */
#define	AT91USART_INT_CTSIC		(1 << 18)	/* Clear to Send Input Change */

/*
 * CSR register bits
 */
#define	AT91USART_CSR_RI		(1 << 20)	/* RI Input */
#define	AT91USART_CSR_DSR		(1 << 21)	/* DSR Input */
#define	AT91USART_CSR_DCD		(1 << 22)	/* DCD Input */
#define	AT91USART_CSR_CTS		(1 << 23)	/* CTS Input */


/*
 * Serial Synchronous Controller (SSC)
 */
#define	AT91RM9200_SSC0_BASE	0xFFFD0000
#define	AT91RM9200_SSC1_BASE	0xFFFD4000
#define	AT91RM9200_SSC2_BASE	0xFFFD8000

/*
 * Serial Peripheral Interface (SPI)
 */
#define	AT91RM9200_SPI_BASE		0xFFFE0000


/*---------------------------------------
            System Peripheral
  ---------------------------------------*/
/*
 * Advanced Interrupt Controller (AIC)
 */
/* AIC Base address */
#define	AT91RM9200_AIC_BASE		0xFFFFF000
#define	AT91RM9200_AIC_SIZE		0x13c

/* AIC Registers, offset from base address */
#define	AT91RM9200_AIC_SMR(x)	(0x04 * (x))
#define	AT91RM9200_AIC_SVR(x)	(0x80 + 0x04 * (x))
#define	AT91RM9200_AIC_IVR		0x100
#define	AT91RM9200_AIC_FVR		0x104
#define	AT91RM9200_AIC_ISR		0x108
#define	AT91RM9200_AIC_IPR		0x10C
#define	AT91RM9200_AIC_IMR		0x110
#define	AT91RM9200_AIC_CISR		0x114
#define	AT91RM9200_AIC_IECR		0x120
#define	AT91RM9200_AIC_IDCR		0x124
#define	AT91RM9200_AIC_ICCR		0x128
#define	AT91RM9200_AIC_ISCR		0x12C
#define	AT91RM9200_AIC_EOICR	        0x130
#define	AT91RM9200_AIC_SPU		0x134
#define	AT91RM9200_AIC_DCR		0x138

/****************************************************************************
 * Debug port
 ****************************************************************************/
#define	AT91RM9200_DBGU_BASE	0xFFFFF200
#define AT91RM9200_DBGU_SIZE    0x4c

/* Register Offsets*/
#define AT91RM9200_DBGU_CR 	0x00	// Control Register
#define AT91RM9200_DBGU_MR 	0x04	// Mode Register
#define AT91RM9200_DBGU_IER 	0x08	// Interrupt Enable Register
#define AT91RM9200_DBGU_IDR 	0x0C	// Interrupt Disable Register
#define AT91RM9200_DBGU_IMR 	0x10	// Interrupt Mask Register
#define AT91RM9200_DBGU_CSR 	0x14	// Channel Status Register
#define AT91RM9200_DBGU_RHR 	0x18	// Receiver Holding Register
#define AT91RM9200_DBGU_THR 	0x1C	// Transmitter Holding Register
#define AT91RM9200_DBGU_BRGR 	0x20	// Baud Rate Generator Register
#define AT91RM9200_DBGU_CIDR 	0x40	// Chip ID Register
#define AT91RM9200_DBGU_EXID 	0x44	// Chip ID2 Register
#define AT91RM9200_DBGU_FNTR 	0x48	// Force NTRST Register

/* bits definition */

  /* (DBGU) */
    /* CR */
#define AT91RM9200_DBGU_CR_RSTSTA                  0x00000100
#define AT91RM9200_DBGU_CR_TXDIS                   0x00000080
#define AT91RM9200_DBGU_CR_TXEN                    0x00000040
#define AT91RM9200_DBGU_CR_RXDIS                   0x00000020
#define AT91RM9200_DBGU_CR_RXEN                    0x00000010
#define AT91RM9200_DBGU_CR_RSTTX                   0x00000008
#define AT91RM9200_DBGU_CR_RSTRX                   0x00000004

    /* MR */
#define AT91RM9200_DBGU_MR_CHMODE(mode)            ((mode & 3) << 14)
    #define AT91RM9200_DBGU_MR_CHMODE_MASK             AT91RM9200_DBGU_MR_CHMODE(3)
    #define AT91RM9200_DBGU_MR_CHMODE_NORMAL           AT91RM9200_DBGU_MR_CHMODE(0)
    #define AT91RM9200_DBGU_MR_CHMODE_AUTO_ECHO        AT91RM9200_DBGU_MR_CHMODE(1)
    #define AT91RM9200_DBGU_MR_CHMODE_LOCAL_LOOPB      AT91RM9200_DBGU_MR_CHMODE(2)
    #define AT91RM9200_DBGU_MR_CHMODE_REMOTE_LOOPB     AT91RM9200_DBGU_MR_CHMODE(3)

#define AT91RM9200_DBGU_MR_PAR(par)                ((par  & 7) << 9)
    #define AT91RM9200_DBGU_MR_PAR_MASK               AT91RM9200_DBGU_MR_PAR(7)
    #define AT91RM9200_DBGU_MR_PAR_EVEN               AT91RM9200_DBGU_MR_PAR(0)
    #define AT91RM9200_DBGU_MR_PAR_ODD                AT91RM9200_DBGU_MR_PAR(1)
    #define AT91RM9200_DBGU_MR_PAR_SPACE              AT91RM9200_DBGU_MR_PAR(2)
    #define AT91RM9200_DBGU_MR_PAR_MARK               AT91RM9200_DBGU_MR_PAR(3)
    #define AT91RM9200_DBGU_MR_PAR_NONE               AT91RM9200_DBGU_MR_PAR(4)

    /* IER, IDR, IMR & SR*/
#define AT91RM9200_DBGU_IE_SR_COMMRX               0x80000000
#define AT91RM9200_DBGU_IE_SR_COMMTX               0x40000000
#define AT91RM9200_DBGU_IE_SR_RXBUFF               0x00001000
#define AT91RM9200_DBGU_IE_SR_TXBUFFE              0x00000800
#define AT91RM9200_DBGU_IE_SR_TXEMPTY              0x00000200
#define AT91RM9200_DBGU_IE_SR_PARE                 0x00000080
#define AT91RM9200_DBGU_IE_SR_FRAME                0x00000040
#define AT91RM9200_DBGU_IE_SR_OVERE                0x00000020
#define AT91RM9200_DBGU_IE_SR_ENDTX                0x00000010
#define AT91RM9200_DBGU_IE_SR_ENDRX                0x00000008
#define AT91RM9200_DBGU_IE_SR_TXRDY                0x00000002
#define AT91RM9200_DBGU_IE_SR_RXRDY                0x00000001

    /* RHR & THR */
#define AT91RM9200_DBGU_RHR_THR_CHAR_MASK          0x000000ff

    /* BRGR */
#define AT91RM9200_DBGU_BRGR_CD_MASK               0x0000ffff
#define AT91RM9200_DBGU_BRGR_CD_DISABLE            0x00000000
#define AT91RM9200_DBGU_BRGR_CD_MCK                0x00000001

    /* CIDR */
#define AT91RM9200_DBGU_CIDR_EXT                   0x80000000
#define AT91RM9200_DBGU_CIDR_NVPTYP                0x70000000
#define AT91RM9200_DBGU_CIDR_ARCH                  0x0ff00000
#define AT91RM9200_DBGU_CIDR_SRAMSIZ               0x000f0000
#define AT91RM9200_DBGU_CIDR_NVPSIZ2               0x0000f000
#define AT91RM9200_DBGU_CIDR_NVPSIZ                0x00000f00
#define AT91RM9200_DBGU_CIDR_EPROC                 0x000000e0
#define AT91RM9200_DBGU_CIDR_VERSION               0x0000001f

    /* EXID */
#define AT91RM9200_DBGU_EXID_MASK                  0xffffffff

    /* FNR */
#define AT91RM9200_DBGU_FNR_FNTRST                 0x00000001








/*
 * PIO A,B,C,D
 */
#define	AT91RM9200_PIOA_BASE	0xFFFFF400
#define	AT91RM9200_PIOB_BASE	0xFFFFF600
#define	AT91RM9200_PIOC_BASE	0xFFFFF800
#define	AT91RM9200_PIOD_BASE	0xFFFFFA00

/* PIO Registers, offset from base address */
#define	AT91RM9200_PIO_PER		0x00
#define	AT91RM9200_PIO_PDR		0x04
#define	AT91RM9200_PIO_PSR		0x08
#define	AT91RM9200_PIO_OER		0x10
#define	AT91RM9200_PIO_ODR		0x14
#define	AT91RM9200_PIO_OSR		0x18
#define	AT91RM9200_PIO_IFER		0x20
#define	AT91RM9200_PIO_IFDR		0x24
#define	AT91RM9200_PIO_IFSR		0x28
#define	AT91RM9200_PIO_SODR		0x30
#define	AT91RM9200_PIO_CODR		0x34
#define	AT91RM9200_PIO_ODSR		0x38
#define	AT91RM9200_PIO_PDSR		0x3C
#define	AT91RM9200_PIO_IER		0x40
#define	AT91RM9200_PIO_IDR		0x44
#define	AT91RM9200_PIO_IMR		0x48
#define	AT91RM9200_PIO_ISR		0x4C
#define	AT91RM9200_PIO_MDER		0x50
#define	AT91RM9200_PIO_MDDR		0x54
#define	AT91RM9200_PIO_MDSR		0x58
#define	AT91RM9200_PIO_PUDR		0x60
#define	AT91RM9200_PIO_PUER		0x64
#define	AT91RM9200_PIO_PUSR		0x68
#define	AT91RM9200_PIO_ASR		0x70
#define	AT91RM9200_PIO_BSR		0x74
#define	AT91RM9200_PIO_ABSR		0x78
#define	AT91RM9200_PIO_OWER		0xA0
#define	AT91RM9200_PIO_OWDR		0xA4
#define	AT91RM9200_PIO_OWSR		0xA8


/*
 * Power Management Controller (PMC)
 */
#define	AT91RM9200_PMC_BASE		0xFFFFFC00

/* PMC Registers, offset from base address */
#define	AT91RM9200_PMC_SCER		0x00
#define	AT91RM9200_PMC_SCDR		0x04
#define	AT91RM9200_PMC_SCSR		0x08
#define	AT91RM9200_PMC_PCER		0x10
#define	AT91RM9200_PMC_PCDR		0x14
#define	AT91RM9200_PMC_PCSR		0x18
#define	AT91RM9200_CKGR_MOR		0x20
#define	AT91RM9200_CKGR_MCFR	0x24
#define	AT91RM9200_CKGR_PLLAR	0x28
#define	AT91RM9200_CKGR_PLLBR	0x2C
#define	AT91RM9200_PMC_MCKR		0x30
#define	AT91RM9200_PMC_PCK0		0x40
#define	AT91RM9200_PMC_PCK1		0x44
#define	AT91RM9200_PMC_PCK2		0x48
#define	AT91RM9200_PMC_PCK3		0x4C
#define	AT91RM9200_PMC_IER		0x60
#define	AT91RM9200_PMC_IDR		0x64
#define	AT91RM9200_PMC_SR		0x68
#define	AT91RM9200_PMC_IMR		0x6C


/*
 * System Timer
 */
#define	AT91RM9200_ST_BASE		0xFFFFFD00

#define	AT91RM9200_ST_SIZE		0x28

/* ST Registers, offset from base address */
#define	AT91RM9200_ST_CR		0x00
#define	AT91RM9200_ST_PIMR		0x04
#define	AT91RM9200_ST_WDMR		0x08
#define	AT91RM9200_ST_RTMR		0x0C
#define	AT91RM9200_ST_SR		0x10
#define	AT91RM9200_ST_IER		0x14
#define	AT91RM9200_ST_IDR		0x18
#define	AT91RM9200_ST_IMR		0x1C
#define	AT91RM9200_ST_RTAR		0x20
#define	AT91RM9200_ST_CRTR		0x24

/*
 * WDMR register bits
 */
#define	AT91RM9200_WDMR_RSTEN	(1 << 16)
#define	AT91RM9200_WDMR_EXTEN	(1 << 17)

/*
 * Real Time Clock (RTC)
 */
#define	AT91RM9200_RTC_BASE		0xFFFFFE00

/*
 * Memory Controller
 */
#define	AT91RM9200_MC_BASE		0xFFFFFF00

/*
 * EMAC
 * Registers, offset from base address
 */


#define AT91RM9200_CTL      0x00    /* Control Register                     */
#define AT91RM9200_CFG      0x04    /* Configuration Register               */
#define AT91RM9200_SR       0x08    /* Status Register                      */
#define AT91RM9200_TAR      0x0C    /* Transmit Address Register            */
#define AT91RM9200_TCR      0x10    /* Transmit Control Register            */
#define AT91RM9200_TSR      0x14    /* Transmit Status Register             */
#define AT91RM9200_RBQP     0x18    /* Receive Buffer Queue Pointer         */
/*                       0x1c       Reserved                             */
#define AT91RM9200_RSR      0x20    /* Receive Status Register              */
#define AT91RM9200_ISR      0x24    /* Interrupt Status Register            */
#define AT91RM9200_IER      0x28    /* Interrupt Enable Register (WO)       */
#define AT91RM9200_IDR      0x2C    /* Interrupt Disable Register (WO)      */
#define AT91RM9200_IMR      0x30    /* Interrupt Mask Register (RO)         */
#define AT91RM9200_MAN      0x34    /* PHY Maintenance Register             */
#define AT91RM9200_FRA      0x40    /* Frames Trasmitted OK Register        */
#define AT91RM9200_SCOL     0x44    /* Single Collision Frame Register      */
#define AT91RM9200_MCOL     0x48    /* Multi Collision Frame Register       */
#define AT91RM9200_OK       0x4C    /* Frames Received OK Register          */
#define AT91RM9200_SEQE     0x50    /* Frames Check Sequence Error Register */
#define AT91RM9200_ALE      0x54    /* Alignment Error Register             */
#define AT91RM9200_DTE      0x58    /* Deferred Transmission Frame Register */
#define AT91RM9200_LCOL     0x5C    /* Late Collision Register              */
#define AT91RM9200_ECOL     0x60    /* Excessive Collision Register         */
#define AT91RM9200_TUE      0x64    /* Transmit Underrun Error Register     */
#define AT91RM9200_CSE      0x68    /* Carrier Sense Error Register         */
#define AT91RM9200_DRFC     0x6C    /* Discard RX Frame Register            */
#define AT91RM9200_ROV      0x70    /* Receive Overrun Register             */
#define AT91RM9200_CDE      0x74    /* Code Error Register                  */
#define AT91RM9200_ELR      0x78    /* Excessive Length Error Register      */
#define AT91RM9200_RJB      0x7C    /* Receive Jabber Register              */
#define AT91RM9200_USF      0x80    /* Undersize Frame Register             */
#define AT91RM9200_SQEE     0x84    /* SQE Test Error Register              */
#define AT91RM9200_HSL      0x90    /* Hash Address Low                     */
#define AT91RM9200_HSH      0x94    /* Hash Address High                    */
#define AT91RM9200_SA1L     0x98    /* Specific Address 1 Low               */
#define AT91RM9200_SA1H     0x9C    /* Specific Address 1 High              */
#define AT91RM9200_SA2L     0xA0    /* Specific Address 2 Low               */
#define AT91RM9200_SA2H     0xA4    /* Specific Address 2 High              */
#define AT91RM9200_SA3L     0xA8    /* Specific Address 3 Low               */
#define AT91RM9200_SA3H     0xAC    /* Specific Address 3 High              */
#define AT91RM9200_SA4L     0xB0    /* Specific Address 4 Low               */
#define AT91RM9200_SA4H     0xB4    /* Specific Address 4 High              */

/* EMAC control register bits defination */
#define AT91RM9200_CTL_LB       (1 << 0)
#define AT91RM9200_CTL_LBL      (1 << 1)
#define AT91RM9200_CTL_RE       (1 << 2)
#define AT91RM9200_CTL_TE       (1 << 3)
#define AT91RM9200_CTL_MPE      (1 << 4)
#define AT91RM9200_CTL_CSR      (1 << 5)
#define AT91RM9200_CTL_ISR      (1 << 6)
#define AT91RM9200_CTL_WES      (1 << 7)
#define AT91RM9200_CTL_BP       (1 << 8)

/* EMAC configuration register bits defination */
#define AT91RM9200_CFG_SPD100   (1 << 0)
#define AT91RM9200_CFG_FD       (1 << 1)
#define AT91RM9200_CFG_BR       (1 << 2)
#define AT91RM9200_CFG_CAF      (1 << 4)
#define AT91RM9200_CFG_NBC      (1 << 5)
#define AT91RM9200_CFG_MTI      (1 << 6)
#define AT91RM9200_CFG_UNI      (1 << 7)
#define AT91RM9200_CFG_BIG      (1 << 8)
#define AT91RM9200_CFG_EAE      (1 << 9)
#define AT91RM9200_CFG_CLK8     (0 << 10)
#define AT91RM9200_CFG_CLK16    (1 << 10)
#define AT91RM9200_CFG_CLK32    (2 << 10)
#define AT91RM9200_CFG_CLK64    (3 << 10)
#define AT91RM9200_CFG_RTY      (1 << 12)
#define AT91RM9200_CFG_PAE      (1 << 13)

/* EMAC status register bits defination */
#define AT91RM9200_SR_MDIO      (1 << 1)
#define AT91RM9200_SR_IDLE      (1 << 2)

/* EMAC receive status register bits definition */
#define AT91RM9200_RSR_BNA      (1 << 0)        /* Buffer Not Available */
#define AT91RM9200_RSR_REC      (1 << 1)        /* Frame Received */
#define AT91RM9200_RSR_OVR      (1 << 2)        /* Receive Overrun */

/* EMAC Interrupt status/enable/disable/mask register bits defination */
#define AT91RM9200_DONE         (1 << 0)        /* Management Frame Done */
#define AT91RM9200_RCOM         (1 << 1)        /* Receive Complete */
#define AT91RM9200_RXUBR        (1 << 2)        /* Receive Used Bit Read */
#define AT91RM9200_TXUBR        (1 << 3)        /* Transmit Used Bit Read */
#define AT91RM9200_TUND         (1 << 4)        /* Transmit Buffer Underrun */
#define AT91RM9200_RTRY         (1 << 5)        /* Retry Limit Exceeded */
#define AT91RM9200_TXERR        (1 << 6)        /* Transmit Error */
#define AT91RM9200_TCOM         (1 << 7)        /* Transmit Complete */
#define AT91RM9200_ROVR         (1 << 10)       /* Receive Overrun */
#define AT91RM9200_HRESP        (1 << 11)       /* Hresp not OK */

/* EMAC transmit status register bits defination */
#define AT91RM9200_TSR_UBR      (1 << 0)        /* Used Bit Read */

#define AT91RM9200_TSR_COL      (1 << 1)        /* Collision Occurred */
#define AT91RM9200_TSR_RLE      (1 << 2)        /* Retry limit exceed */
#define AT91RM9200_TSR_TGO      (1 << 3)        /* Transmit Go */
#define AT91RM9200_TSR_BEX      (1 << 4)        /* Buffers exhausted mid frame */
#define AT91RM9200_TSR_COMP     (1 << 5)        /* Transmit Complete */
#define AT91RM9200_TSR_UND      (1 << 6)        /* Transmit Underrun */

/* EMAC User Input/Output Register bits definition */
#define AT91RM9200_USRIO_RMII       (1 << 0)
#define AT91RM9200_USRIO_CLKEN      (1 << 1)


#endif	/* __ARM_AT91RM9200_H_INCLUDED */

/* __SRCVERSION("at91rm9200.h $Rev: 169789 $"); */
