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




// Module Description:  MMC header file

#ifndef _MMC_H_INCLUDED
#define _MMC_H_INCLUDED

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <atomic.h>
#include <pthread.h>
#include <gulliver.h>
#include <sys/mman.h>
#include <sys/disk.h>
#include <sys/types.h>
#include <sys/resmgr.h>
#include <sys/syspage.h>
#include <sys/dcmd_cam.h>
#include <hw/dcmd_sim_mmcsd.h>

// CAM specific includes
#include <module.h>
#include <ntocam.h>
#include <sim.h>

#include <mmcsd.h>

#define	_SLOGC_SIM_MMC			__C(_SLOGC_SIM, 1000)

#define MMCARG_VAL( _o, _v ) if( (_v) == NULL || *(_v) == '\0' ) { fprintf( stderr, "mmc_sim_args: Missing argument for '%s'", opts[ _o ] ); return( EINVAL ); }
#define MMC_MAX_TARGET		1
#define MMC_MAX_HBA			1
#define MMC_MAX_SG			128

#define MMC_DFLT_CAPACITY	16384		// default to 8MB
#define MMC_DFLT_BLKSIZE	512

// SCSI spindle actions
#define MMC_SPINDLE_STOP	0
#define MMC_SPINDLE_START	1
#define MMC_SPINDLE_EJECT	2
#define MMC_SPINDLE_LOAD	3

#define MMC_CFLAG_SCAN		0x01	// auto detect interfaces
#define MMC_CFLAG_DBG		0x80
typedef struct _mmc_ctrl {
	TAILQ_HEAD(,_sim_hba)	hlist;		// linked list of hba's
	uint32_t				cflags;
	uint32_t				nhba;					// number of hba's
	uint32_t				pathid_max;				// max path id
} MMC_CTRL;

#define MMC_EFLAG_INITIALIZED		0x01
#define MMC_EFLAG_VALID				0x02
#define MMC_EFLAG_MEDIA_CHANGED     0x04
#define	MMC_EFLAG_NODMA				0x08
#define	MMC_EFLAG_SG				0x10
typedef struct _sim_mmc_ext {
	SIM_HBA				*sim;
	uint32_t			eflags;
	CCB_SCSIIO			*nexus;
	_uint32				mmc_c_size;
	_uint32				mmc_c_size_m;
	_uint32				mmc_blksize;
	char				*bsopts;
	void				*bshdl;
} SIM_MMC_EXT;

int mmc_sim_args( char *options );
int mmc_sim_detach( void );
int mmc_sim_attach( CAM_ENTRY *centry );
long mmc_sim_init( SIM_HBA *hba, long path );
long mmc_sim_action( SIM_HBA *hba, CCB *ccb_ptr );
void *mmc_driver_thread( void *data );
void mmc_io_cmds( SIM_HBA *hba );

#ifndef EXTERN
#define EXTERN_ADDED
#define EXTERN extern
#define	VALUE(x)
#else
#define VALUE(x) = { x }
#endif

EXTERN MMC_CTRL mmc_ctrl
#ifndef EXTERN_ADDED
= { { 0 },0 }
#endif
;

#ifdef EXTERN_ADDED
#undef EXTERN_ADDED
#undef EXTERN
#endif
#undef VALUE

#endif

/* __SRCVERSION("sim_mmc.h $Rev: 168632 $"); */
