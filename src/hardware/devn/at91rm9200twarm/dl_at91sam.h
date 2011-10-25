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

 
#ifndef __DL_AT91SAM_H__
#define __DL_AT91SAM_H__

extern int io_net_dll_entry_at91sam();

static const struct dll_syms _at91sam_symbols[] = {
	{"io_net_dll_entry", &io_net_dll_entry_at91sam},
	{NULL, NULL}
};

#define DLL_AT91sam_LIST		"devn-at91sam.so", _at91sam_symbols


#endif
