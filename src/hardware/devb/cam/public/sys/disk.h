/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
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
 *  sys/disk.h
 *
 */
#ifndef __DISK_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include _NTO_HDR_(_pack64.h)

__BEGIN_DECLS

typedef struct partition_entry {
    unsigned char   boot_ind,
                    beg_head,
                    beg_sector,
                    beg_cylinder,
                    os_type,
                    end_head,
                    end_sector,
                    end_cylinder;
    unsigned long   part_offset,
                    part_size;
} partition_entry_t;


__END_DECLS

#include _NTO_HDR_(_packpop.h)

#define __DISK_H_INCLUDED
#endif

/* __SRCVERSION("$IQ$"); */
