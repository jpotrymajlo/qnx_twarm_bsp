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
 * AT91RM9200 specific timer support.
 *
 * This should be usable by any board that uses an AT91RM9200
 */

#include "startup.h"
#include <arm/at91rm9200.h>

/* based on slow clock for real timer couner */
#define    AT91RM9200TWARM_CLOCK_FREQ         32768UL
#define    AT91RM9200TWARM_CLOCK_RATE         3051757813UL
#define    AT91RM9200TWARM_CLOCK_SCALE        -14


extern struct callout_rtn	timer_load_at91rm9200;
extern struct callout_rtn	timer_value_at91rm9200;
extern struct callout_rtn	timer_reload_at91rm9200;

static uintptr_t timer_base;
static unsigned timer_start_value;

static const struct callout_slot	timer_callouts[] = {
	{ CALLOUT_SLOT(timer_load, _at91rm9200) },
	{ CALLOUT_SLOT(timer_value, _at91rm9200) },
	{ CALLOUT_SLOT(timer_reload, _at91rm9200) },
};

static unsigned timer_start_at91rm9200()
{
    timer_start_value = in32(timer_base + AT91RM9200_ST_CRTR) & 0x000FFFFF;
    return timer_start_value;
}

static unsigned timer_diff_at91rm9200(unsigned start)
{
	unsigned current_value = in32(timer_base + AT91RM9200_ST_CRTR);
	return (current_value - timer_start_value) & 0x000FFFFF;
}

void init_qtime_at91rm9200()
{
    struct qtime_entry  *qtime = alloc_qtime();

    /*
     * Map the timer 0 registers
     */

    timer_base = startup_io_map(AT91RM9200_ST_SIZE, AT91RM9200_ST_BASE);

    /*
     * Disable PIT and RTT
     */
    out32(timer_base + AT91RM9200_ST_IDR, 0x01 | 0x04);

        /*
         * Count every period of CLCK in RTT
         */
    out32(timer_base + AT91RM9200_ST_RTMR, 0x01);

    timer_start = timer_start_at91rm9200;
    timer_diff  = timer_diff_at91rm9200;

    qtime->intr = 1;

    qtime->timer_rate     = AT91RM9200TWARM_CLOCK_RATE;
    qtime->timer_scale    = AT91RM9200TWARM_CLOCK_SCALE;
    qtime->cycles_per_sec = (uint64_t)AT91RM9200TWARM_CLOCK_FREQ;

    add_callout_array(timer_callouts, sizeof(timer_callouts));
}
