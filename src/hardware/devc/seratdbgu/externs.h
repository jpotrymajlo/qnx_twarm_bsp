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




#ifdef DEFN
	#define EXT
	#define INIT1(a)				= { a }
#else
	#define EXT extern
	#define INIT1(a)
#endif

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <atomic.h>
#include <hw/inout.h>
#include <sys/io-char.h>

#include <arm/at91rm9200.h>

typedef struct dev_usart {
	TTYDEV		tty;
	unsigned	intr;
	int			iid;
	unsigned	clk;
	uintptr_t	base;
	unsigned	baud;
	unsigned	mode;
	unsigned	txe;
} DEV_USART;

typedef struct ttyinit_usart {
	TTYINIT		tty;
} TTYINIT_USART;

EXT TTYCTRL				ttyctrl;

#include "proto.h"
