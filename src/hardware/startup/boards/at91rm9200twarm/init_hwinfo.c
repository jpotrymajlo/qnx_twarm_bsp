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





#include "startup.h"
#include <board.h>

#define MAX_BOARDS	29

typedef struct _board_info {
	uint32_t cpu_id;
	char board_name[15];
} board_info;

board_info board_list[MAX_BOARDS] = { {0x170A0940,"AT91SAM7A3"},
			{0x260A0940, "AT91SAM7A3"},
			{0x27280340, "AT91SAM7S321"},
			{0x27080340, "AT91SAM7S32"},
			{0x27090540, "AT91SAM7S64"},
			{0x270A0740, "AT91SAM7S128"},
			{0x270C0740, "AT91SAM7S128"},
			{0x270B0940, "AT91SAM7S256"},
			{0x270D0940, "AT91SAM7S256"},
			{0x270B0A40, "AT91SAM7S512"},
			{0x27280340, "AT91SAM7SE32"},
			{0x272A0940, "AT91SAM7SE256"},
			{0x272A0A40, "AT91SAM7SE512"},
			{0x275A0740, "AT91SAM7X128"},
			{0x275B0940, "AT91SAM7X256"},
			{0x275A0A40, "AT91SAM7X512"},
			{0x271A0740, "AT91SAM7XC128"},
			{0x271B0940, "AT91SAM7XC256"},
			{0x271C0A40, "AT91SAM7XC512"},
			{0x019803A0, "AT91SAM9260"},
			{0x019703A0, "AT91SAM9261"},
			{0x019607A0, "AT91SAM9263"},
			{0x329973A0, "AT91SAM9XE128"},
			{0x329A93A0, "AT91SAM9XE256"},
			{0x329AA3A0, "AT91SAM9XE512"},
			{0x019B03A0, "AT91SAM9RL64"},
			{0x039A03A0, "AT91CAP9"},
			{0x09290781, "AT91RM9200"},
			{0x0, "UNKNOWN"}
};

/*
 * get board name for the corresponding cpu id
 */

char * get_board_name()
{
	int i;
	uint32_t cpu_id;

	cpu_id = in32(AT91RM9200_DBGU_BASE + AT91RM9200_DBGU_CIDR);

	for(i=0 ; i < MAX_BOARDS -1; i++)
		if(cpu_id == board_list[i].cpu_id)
			return board_list[i].board_name;

	return  board_list[MAX_BOARDS - 1].board_name;
}


/*
 * Initialize hwinfo structure in the system page.
 * This code is hardware dependant and may have to be changed
 * changed by end users.
 */

void
init_hwinfo()
{
		set_syspage_section(&lsp.cpu.arm_boxinfo, sizeof(*lsp.cpu.arm_boxinfo.p));
}

