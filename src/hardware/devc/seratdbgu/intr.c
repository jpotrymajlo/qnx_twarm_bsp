/*
 * $QNXLicenseC: 
 * Copyright 2008 QNX Software Systems Gmbh & Co. KG. All rights reserved. 
 * This software may not be used in, licensed for use with or otherwise 
 * distributed for use with oxymeter products. It is otherwise licensed 
 * under the Apache License, Version 2.0 (the "License"); you may not use 
 * this file except in compliance with the License. You may obtain a copy 
 * of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless 
 * required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES 
 * OR CONDITIONS OF ANY KIND, either express or implied. See the License for 
 * the specific language governing permissions and limitations under the 
 * License. 
 * $
 */


#include "externs.h"

static inline int tx_interrupt(DEV_USART *dev)
{
	dev->tty.un.s.tx_tmr = 0;	/* clear Timeout */

	return tto(&dev->tty, TTO_DATA, 0);
}

static inline int rx_interrupt(DEV_USART *dev)
{
	int			csr, status = 0;
	uintptr_t	base = dev->base;

	while ((csr = in32(base + AT91RM9200_DBGU_CSR)) & USART_RXEVENT) {
		unsigned    key = 0;

		/*
		 * Read next character from FIFO
		 */
		if (csr & AT91RM9200_DBGU_IE_SR_RXRDY)
			key = in32(base + AT91RM9200_DBGU_RHR);

		if (csr & USART_RXERR) {
			/*
			 * Save error as out-of-band data which can be read via devctl()
			 */
			dev->tty.oband_data |= csr;
			atomic_set(&dev->tty.flags, OBAND_DATA);

			if (csr & AT91RM9200_DBGU_IE_SR_FRAME)
				key |= TTI_FRAME;
			else if (csr & AT91RM9200_DBGU_IE_SR_PARE)
				key |= TTI_PARITY;
			else if (csr & AT91RM9200_DBGU_IE_SR_OVERE)
				key |= TTI_OVERRUN;

			out32(base + AT91RM9200_DBGU_CR, AT91RM9200_DBGU_CR_RSTSTA);
		}

		status |= tti(&dev->tty, key);
	}

	return status;
}

static inline int do_interrupt(DEV_USART *dev, int id)
{
	int	sts;

	sts = rx_interrupt(dev);
	if (in32(dev->base + AT91RM9200_DBGU_CSR) & AT91RM9200_DBGU_IE_SR_TXRDY)
	    sts |= tx_interrupt(dev);

	return sts;
}

/*
 * Serial interrupt handler
 */
static const struct sigevent * ser_intr(void *area, int id)
{
	DEV_USART	*dev = area;

	if (do_interrupt(dev, id) && (dev->tty.flags & EVENT_QUEUED) == 0) {
		dev_lock(&ttyctrl);
		ttyctrl.event_queue[ttyctrl.num_events++] = &dev->tty;
		atomic_set(&dev->tty.flags, EVENT_QUEUED);
		dev_unlock(&ttyctrl);
		return &ttyctrl.event;
	}

	return 0;
}

void
ser_attach_intr(DEV_USART *dev)
{
	uintptr_t	base = dev->base;

	/* Disable all interrupts */
	out32(dev->base + AT91RM9200_DBGU_IDR, USART_INTR);

	/* Disable Tx/Rx, reset status bits */
	out32(base + AT91RM9200_DBGU_CR, AT91RM9200_DBGU_CR_RXDIS | AT91RM9200_DBGU_CR_TXDIS | AT91RM9200_DBGU_CR_RSTSTA);

	dev->iid = InterruptAttach(dev->intr, ser_intr, dev, 0, _NTO_INTR_FLAGS_TRK_MSK);
}

