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



#ifndef		_MMC_INCLUDED
#define		_MMC_INCLUDED

/* MultiMediaCard Command definitions */
/* An 'S' after the definition means that this command is available in SPI mode */
/* SPI commands are a subset of the MMC definition */

#define	MMC_GO_IDLE_STATE			0		/* S */
#define	MMC_SEND_OP_COND			1		/* S */
#define	MMC_ALL_SEND_CID			2
#define	MMC_SET_RELATIVE_ADDR		3
#define	MMC_SET_DSR					4
#define	MMC_SEL_DES_CARD			7
#define	MMC_IF_COND				8
#define	MMC_SEND_CSD				9		/* S */
#define	MMC_SEND_CID				10		/* S */
#define	MMC_READ_DAT_UNTIL_STOP		11
#define	MMC_STOP_TRANSMISSION		12
#define	MMC_SEND_STATUS				13		/* S */
#define	MMC_GO_INACTIVE_STATE		15
#define	MMC_SET_BLOCKLEN			16		/* S */
#define	MMC_READ_SINGLE_BLOCK		17		/* S */
#define	MMC_READ_MULTIPLE_BLOCK		18
#define	MMC_WRITE_DAT_UNTIL_STOP	20
#define	MMC_WRITE_BLOCK				24		/* S */
#define	MMC_WRITE_MULTIPLE_BLOCK	25
#define	MMC_PROGRAM_CID				26
#define	MMC_PROGRAM_CSD				27		/* S */
#define	MMC_SET_WRITE_PROT			28		/* S */
#define	MMC_CLR_WRITE_PROT			29		/* S */
#define	MMC_SEND_WRITE_PROT			30		/* S */
#define	MMC_TAG_SECTOR_START		32		/* S */
#define	MMC_TAG_SECTOR_END			33		/* S */
#define	MMC_UNTAG_SECTOR			34		/* S */
#define	MMC_TAG_ERASE_GROUP_START	35		/* S */
#define	MMC_TAG_ERASE_GROUP_END		36		/* S */
#define	MMC_UNTAG_ERASE_GROUP		37		/* S */
#define	MMC_ERASE					38		/* S */
#define	MMC_FAST_IO					39
#define	MMC_GO_IRQ_STATE			40
#define	MMC_LOCK_UNLOCK				42		/* S */
#define	MMC_APP_CMD					55		/* S */
#define	MMC_GEN_CMD					56		/* S */
#define	MMC_READ_OCR				58		/* S */
#define	MMC_CRC_ON_OFF				59		/* S */

#define	SET_BUS_WIDTH				6
#define	SD_SEND_OP_COND				41

/* Card Status Response Bits */

#define	MMC_OUT_OF_RANGE			1 << 31
#define	MMC_ADDRESS_ERROR			1 << 30
#define	MMC_BLOCK_LEN_ERROR			1 << 29
#define	MMC_ERASE_SEQ_ERROR			1 << 28
#define	MMC_ERASE_PARAM				1 << 27
#define	MMC_WP_VIOLATION			1 << 26
#define	MMC_CARD_IS_LOCKED			1 << 25
#define	MMC_LOCK_UNLOCK_FAILED		1 << 24
#define	MMC_COM_CRC_ERROR			1 << 23
#define	MMC_ILLEGAL_COMMAND			1 << 22
#define	MMC_CARD_ECC_FAILED			1 << 21
#define	MMC_CC_ERROR				1 << 20
#define	MMC_ERROR					1 << 19
#define	MMC_UNDERRUN				1 << 18
#define	MMC_OVERRUN					1 << 17
#define	MMC_CID_CSD_OVERWRITE		1 << 16
#define	MMC_WP_ERASE_SKIP			1 << 15
#define	MMC_CARD_ECC_DISABLED		1 << 14
#define	MMC_ERASE_RESET				1 << 13
/* Bits 9-12 define the CURRENT_STATE */
#define	MMC_IDLE					0 << 9
#define	MMC_READY					1 << 9
#define	MMC_IDENT					2 << 9
#define	MMC_STANDBY					3 << 9
#define	MMC_TRAN					4 << 9
#define	MMC_DATA					5 << 9
#define	MMC_RCV						6 << 9
#define	MMC_PRG						7 << 9
#define	MMC_DIS						8 << 9
/* End CURRENT_STATE */
#define	MMC_READY_FOR_DATA			1 << 8
#define	MMC_APP_CMD_S				1 << 5

/* SPI Mode Response R1 format */

#define	MMC_PARAM_ERROR				1 << 6
#define	MMC_SADDRESS_ERROR			1 << 5
#define	MMC_SERASE_SEQ_ERROR		1 << 4
#define	MMC_SCOM_CRC_ERROR			1 << 3
#define	MMC_SILLEGAL_COMMAND		1 << 2
#define	MMC_SERASE_RESET			1 << 1
#define	MMC_IDLE_STATE				1 << 0

/* SPI Mode Response R2 format */
/* First byte is the same as R1 format */

#define	MMC_SOUT_OF_RANGE			1 << 7
#define	MMC_SERASE_PARAM			1 << 6
#define	MMC_SWP_VIOLATION			1 << 5
#define	MMC_SCARD_ECC_FAILED		1 << 4
#define	MMC_SCC_ERROR				1 << 3
#define	MMC_SERROR					1 << 2
#define	MMC_SWP_ERASE_SKIP			1 << 1
#define	MMC_SCARD_IS_LOCKED			1 << 0

/* I/O Definitions for target ID */

#define	MMC_TARGET_MMC				0
#define	MMC_TARGET_MAS				1

/* I/O Flag definitions */

#define	MMC_DIR_NONE			0
#define	MMC_DIR_IN				(1 << 0)
#define	MMC_DIR_OUT				(1 << 1)	
#define	MMC_CRC7				(1 << 2)
#define	MMC_CRC16				(1 << 3)
#define	MMC_DIR_MASK			(MMC_DIR_IN | MMC_DIR_OUT)

/* I/O Status definitions */

#define	MMC_SUCCESS				0
#define	MMC_FAILURE				1
#define	MMC_DATA_OVERRUN		2
#define	MMC_BAD_FLAG			3
#define	MMC_NOT_PRESENT			4
#define	MMC_TIMEOUT				5
#define	MMC_ALLOC_FAILED		6
#define	MMC_INVALID_HANDLE		7
#define	MMC_COMMAND_FAILURE		8
#define	MMC_READ_ERROR			9
#define	MMC_WRITE_ERROR			10

/* MMC I/O structure */

typedef	struct	_mmc_io {
		int			mmc_tid;		/* Target ID */
		int			mmc_data_len;	/* Data length */
		_uint8		*mmc_data;		/* Data/SG buffer */
		_uint32		mmc_flags;		/* Flags */
#define	MMCIO_SG	(1 << 31)		/* Scatter gather buffers */
#define	MMCIO_VIRT	(1 << 30)		/* Virtual address */
		int			mmc_status;		/* Returned status of function */
		_uint8		mmc_cmd;		/* Command to perform */
		_uint32		mmc_arg;		/* Argument of command */
} mmc_io_t;


/* Prototypes */

void	*mmc_attach (void *, char *);
int		mmc_detach (void *);
int		mmc_io (void *hdl, mmc_io_t *mmc);

#endif

/* __SRCVERSION("mmcsd.h $Rev: 168632 $"); */
