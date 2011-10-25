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


#ifndef __PROTO_H__
#define __PROTO_H__
#define	USART_RXERR		( AT91RM9200_DBGU_IE_SR_OVERE \
                        | AT91RM9200_DBGU_IE_SR_FRAME \
                        | AT91RM9200_DBGU_IE_SR_PARE)
#define	USART_RXEVENT	( USART_RXERR | AT91RM9200_DBGU_IE_SR_RXRDY)
#define USART_INTR      ( AT91RM9200_DBGU_IE_SR_RXBUFF  \
                        | AT91RM9200_DBGU_IE_SR_TXBUFFE \
                        | AT91RM9200_DBGU_IE_SR_TXEMPTY \
                        | AT91RM9200_DBGU_IE_SR_PARE    \
                        | AT91RM9200_DBGU_IE_SR_FRAME   \
                        | AT91RM9200_DBGU_IE_SR_OVERE   \
                        | AT91RM9200_DBGU_IE_SR_ENDTX   \
                        | AT91RM9200_DBGU_IE_SR_ENDRX   \
                        | AT91RM9200_DBGU_IE_SR_TXRDY   \
                        | AT91RM9200_DBGU_IE_SR_RXRDY   )


void		create_device(TTYINIT_USART *dip, unsigned unit);
void		ser_stty(DEV_USART *dev);
void		ser_ctrl(DEV_USART *dev, unsigned flags);
void		ser_attach_intr(DEV_USART *dev);
void *		query_default_device(TTYINIT_USART *dip, void *link);
unsigned	options(int argc, char *argv[]);



#endif /* #ifdef __PROTO_H__ */
