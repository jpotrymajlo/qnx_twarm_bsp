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
 *  dcmd_sim_mmcsd.h   Non-portable low-level devctl definitions
 *
*/

#ifndef __DCMD_SIM_MMCSD_H_INCLUDED
#define __DCMD_SIM_MMCSD_H_INCLUDED

#ifndef _DEVCTL_H_INCLUDED
 #include <devctl.h>
#endif

#include <_pack64.h>

#define MMCSD_PARSED_CID	0x0
#define MMCSD_FULL_CID		0x1

 /* CID reg values of the card */
typedef struct _mmcsd_cid {
	_Uint32t		flags;	/*set to 0 - return parsed CID contents, 1 - return full cid contents */
	_Uint8t           rsvd[4];
	union{
		struct{
			_Uint32t            cid[4];
		}full_cid;
		
		struct{
			_Uint32t		psn; 	/* Product Serial Number */
			_Uint16t		oid;		/* OEM/Application ID */
			_Uint16t		mdt; 	/* Manufacturing Date */
			_Uint8t           pnm[5];      /* Product Name */
			_Uint8t		prv;		/* Product Revision */
			_Uint8t		mid; 	/* Manufacturer ID */
			_Uint8t		crc7;	/* CRC7 Checksum */
		}parsed_cid;
	}cid;
} MMCSD_CID;

#define DCMD_MMCSD_GET_CID       	__DIOTF(_DCMD_CAM, _SIM_MMCSD + 0, struct _mmcsd_cid)

#include <_packpop.h>

#endif
