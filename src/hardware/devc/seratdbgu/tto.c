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

int
tto(TTYDEV *ttydev, int action, int arg1)
{
	TTYBUF 		*bup = &ttydev->obuf;
	DEV_USART	*dev = (DEV_USART *)ttydev;
	uintptr_t	base = dev->base;
	int 		status = 0;
	uint8_t		c;

	switch (action) {
	case TTO_STTY:
		ser_stty(dev);
		return 0;
 
	case TTO_CTRL:
		return 0;

	case TTO_LINESTATUS:
		return in32(base + AT91RM9200_DBGU_CR);

	case TTO_DATA:
		break;

	default:
		return 0;
	}

	/*
	 * If the OSW_PAGED_OVERRIDE flag is set then allow
	 * transmit of character even if output is suspended via
	 * the OSW_PAGED flag. This flag implies that the next
	 * character in the obuf is a software flow control
	 * charater (STOP/START).
	 * Note: tx_inject sets it up so that the contol
	 *       character is at the start (tail) of the buffer.
	 */
	if (dev->tty.flags & (OHW_PAGED|OSW_PAGED) && !(dev->tty.xflags & OSW_PAGED_OVERRIDE))
		return 0;


	if (bup->cnt > 0) {
	
		if (in32(base + AT91RM9200_DBGU_CSR) & AT91RM9200_DBGU_IE_SR_TXRDY) {
			dev_lock(&dev->tty);
			/*
			 * Get the next character to print from the output buffer
			 */
			c = *bup->tail;
			if (c == '\n' &&
			    ((dev->tty.c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR)) && 
			    ((dev->tty.flags & NL_INSERT) == 0)) {
				c = '\r';
				atomic_set(&dev->tty.flags, NL_INSERT);
			} else {
				atomic_clr(&dev->tty.flags, NL_INSERT);
				if (++bup->tail >= &bup->buff[bup->size])
					bup->tail = &bup->buff[0];
				--bup->cnt;
			}
	
			dev->tty.un.s.tx_tmr = 3;		/* Timeout 3 */
			out32(base + AT91RM9200_DBGU_THR, c);

			/*
			 * Enable TXRDY interrupt if it's not enabled
			 */
			if (dev->txe != AT91RM9200_DBGU_IE_SR_TXRDY) {
				dev->txe = AT91RM9200_DBGU_IE_SR_TXRDY;
				out32(base + AT91RM9200_DBGU_IER, dev->txe);
			}

			/*
			 * Clear the OSW_PAGED_OVERRIDE flag as we only want
			 * one character to be transmitted in this case.
			 */
			if (dev->tty.xflags & OSW_PAGED_OVERRIDE)
				atomic_clr(&dev->tty.xflags, OSW_PAGED_OVERRIDE);
			dev_unlock(&dev->tty);
		}
	} else {

		/* Disable TXRDY interrupt */
		dev->txe = 0;
		out32(base + AT91RM9200_DBGU_IDR, AT91RM9200_DBGU_IE_SR_TXRDY);

		/*
		 * Check for notify conditions
		 */
		if (dev->tty.notify[1].cnt < bup->size - bup->cnt) {
			dev->tty.notify[1].cnt = (~0u) >> 1;	// Disarm
			atomic_set(&dev->tty.flags, EVENT_NOTIFY_OUTPUT);
			status = 1;
		}

		/*
		 * Is anyone waiting for the output buffer to drain?
		 */
		if (dev->tty.waiting_drain && bup->cnt == 0) {
			atomic_set(&dev->tty.flags, EVENT_DRAIN);
			status = 1;
		}
	}

	

	return status;
}

void ser_stty(DEV_USART *dev)
{
	uintptr_t	base;
	unsigned	mode = 0;

	/*
	 * Charactor length
	 */
	switch (dev->tty.c_cflag & CSIZE) {
	case CS5:
		mode |= 0 << 6; break;

	case CS6:
		mode |= 1 << 6; break;

	case CS7:
		mode |= 2 << 6; break;

	case CS8:
	default:
		mode |= 3 << 6; break;
	}

	/*
	 * 2 stop bit ?
	 */
	if (dev->tty.c_cflag & CSTOPB)
		mode |= 2 << 12;

	/*
	 * Parity
	 */
	if (dev->tty.c_cflag & PARENB) {
		if (dev->tty.c_cflag & PARODD)
			mode |= 1 << 9;
	}
	else
		mode |= 4 << 9;

	if (mode == dev->mode && dev->tty.baud == dev->baud)
		return;

	dev->baud = dev->tty.baud;
	dev->mode = mode;

	base = dev->base;

	/* Disable UART */
	out32(base + AT91RM9200_DBGU_CR, AT91RM9200_DBGU_CR_RXDIS | AT91RM9200_DBGU_CR_TXDIS);

	/* Program mode register and BRG register */
	out32(base + AT91RM9200_DBGU_MR,    mode);
	out32(base + AT91RM9200_DBGU_BRGR, (dev->clk >> 4) / dev->baud);

	/* Enable UART and Receiver Ready Interrupt */
	out32(base + AT91RM9200_DBGU_IER, USART_RXEVENT);

	/* Enable Tx/Rx */
	out32(base + AT91RM9200_DBGU_CR, AT91RM9200_DBGU_CR_RXEN | AT91RM9200_DBGU_CR_TXEN);
}

