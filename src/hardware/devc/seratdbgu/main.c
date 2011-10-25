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
main(int argc, char *argv[])
{
	ttyctrl.max_devs = 5;
	ttc(TTC_INIT_PROC, &ttyctrl, 24);

    if (options(argc, argv) == 0) {
		fprintf(stderr, "%s: No serial ports found\n", argv[0]);
		exit(0);
    }

	ttc(TTC_INIT_START, &ttyctrl, 0);

	return 0;
}
