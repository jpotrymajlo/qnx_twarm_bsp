/*
 * $QNXLicenseC:
 * Copyright 2009, QNX Software Systems.
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

#include <startup.h>
#include <hw/inout.h>
#include <board.h>
#define RSTC_KEY        (0xFF << 24) /* (RSTC) Password */
#define RSTC_ERSTL      (0xF <<  8)  /* (RSTC) User Reset Enable */
#define RSTC_URSTIEN    (0x1 <<  4)  /* (RSTC) User Reset Interrupt Enable */
#define RSTC_EXTRST     (0x1 <<  3)  /* (RSTC) External Reset */
#define RSTC_NRSTL      (0x1 << 16)  /* (RSTC) NRST pin level */

/* Initialize PIO */
void pio_init (void)
{

}
