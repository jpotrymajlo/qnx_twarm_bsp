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



/*
 * AT91RM9200 Evaluation Board
 */

#include "startup.h"
#include <arm/at91rm9200.h>

extern struct callout_rtn 	reboot_at91rm9200;
extern struct callout_rtn	display_char_at91rm9200;
extern struct callout_rtn	poll_key_at91rm9200;
extern struct callout_rtn	break_detect_at91rm9200;

void init_at91rm9200(unsigned, const char *, const char *);
void put_at91rm9200(int);

extern void pio_init (void );
extern char * get_board_name();

const struct callout_slot callouts[] = {
	{ CALLOUT_SLOT( reboot, _at91rm9200 ) },
};

const struct debug_device debug_devices[] = {
	{ 	"at91rm9200",
		{	"0xFFFFF200^0.115200.59904000.16",	/* Use whatever boot loader baud rate */
		},
		init_at91rm9200,
		put_at91rm9200,
		{	&display_char_at91rm9200,
			&poll_key_at91rm9200,
			&break_detect_at91rm9200,
		}
	},
};
/*
 * main()
 *	Startup program executing out of RAM
 *
 * 1. It gathers information about the system and places it in a structure
 *    called the system page. The kernel references this structure to
 *    determine everything it needs to know about the system. This structure
 *    is also available to user programs (read only if protection is on)
 *    via _syspage->.
 *
 * 2. It (optionally) turns on the MMU and starts the next program
 *    in the image file system.
 */
int
main(int argc, char **argv, char **envv)
{
	int	opt;

	add_callout_array(callouts, sizeof(callouts));

	while ((opt = getopt(argc, argv, COMMON_OPTIONS_STRING)) != -1) {
				handle_common_option(opt);
	}

	/*
	 * set CPU frequency
	 */
	if (cpu_freq == 0)
		cpu_freq = 100000000;

	/*
	 * Initialise debugging output
	 */
	select_debug(debug_devices, sizeof(debug_devices));

	/*
	 * Initialize PIO
	 */
//	pio_init();

	/*
	 * Collect information on all free RAM in the system
	 */
	init_raminfo();

	/*
	 * Do board specific initialization
	 */


	/*
	 * Remove RAM used by modules in the image
	 */
	alloc_ram(shdr->ram_paddr, shdr->ram_size, 1);
	if (shdr->flags1 & STARTUP_HDR_FLAGS1_VIRTUAL)
		init_mmu();

	init_intrinfo();
	init_qtime_at91rm9200();
	init_cacheattr();
	init_cpuinfo();
	init_hwinfo();

	add_typed_string(_CS_MACHINE, (const char *)get_board_name());

	/*
	 * Load bootstrap executables in the image file system and Initialise
	 * various syspage pointers. This must be the _last_ initialisation done
	 * before transferring control to the next program.
	 */
	init_system_private();

	/*
	 * This is handy for debugging a new version of the startup program.
	 * Commenting this line out will save a great deal of code.
	 */
	print_syspage();

	return 0;
}
