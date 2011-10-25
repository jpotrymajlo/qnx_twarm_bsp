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
 * AT91RM9200 Advanced Interrupt Controller (AIC) support.
 */

#include "startup.h"
#include <arm/at91rm9200.h>

extern struct callout_rtn	interrupt_id_at91rm9200_aic;
extern struct callout_rtn	interrupt_eoi_at91rm9200_aic;
extern struct callout_rtn	interrupt_mask_at91rm9200_aic;
extern struct callout_rtn	interrupt_unmask_at91rm9200_aic;

static paddr_t at91rm9200_aic_base = AT91RM9200_AIC_BASE;

const static struct startup_intrinfo	intrs[] = {
	{	_NTO_INTR_CLASS_EXTERNAL, 	// vector base
		32,							// number of vectors
		_NTO_INTR_SPARE,			// cascade vector
		0,							// CPU vector base
		0,							// CPU vector stride
		0,							// flags

		{ INTR_GENFLAG_LOAD_SYSPAGE,	0, &interrupt_id_at91rm9200_aic },
		{ INTR_GENFLAG_LOAD_SYSPAGE | INTR_GENFLAG_LOAD_INTRMASK, 0, &interrupt_eoi_at91rm9200_aic },
		&interrupt_mask_at91rm9200_aic,	// mask   callout
		&interrupt_unmask_at91rm9200_aic,	// unmask callout
		0,							// config callout
		&at91rm9200_aic_base,
	},
};



void init_intrinfo()
{
	int	i;

	/* Disable all the interupts */
	out32(AT91RM9200_AIC_BASE + AT91RM9200_AIC_IDCR, ~0);

	/* Clear all the interupts */
	out32(AT91RM9200_AIC_BASE + AT91RM9200_AIC_ICCR, ~0);

	/* Program mode registers */
	for (i = 0; i < 32; i++)
		out32(AT91RM9200_AIC_BASE + AT91RM9200_AIC_SMR(i), 0);

	/* Program vector registers */
	for (i = 0; i < 32; i++)
		out32(AT91RM9200_AIC_BASE + AT91RM9200_AIC_SVR(i), i);

	add_interrupt_array(intrs, sizeof(intrs));
}
