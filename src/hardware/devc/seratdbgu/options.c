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


/*
#ifdef __USAGE
%C - Serial driver for AT91RM9200 DBGU

%C [options] [port,irq] &
Options:
 -b number    Define initial baud rate (default 115200)
 -c clk       Set the input clock rate (default 59904000)
 -C number    Size of canonical input buffer (default 256)
 -e           Set options to "edit" mode
 -E           Set options to "raw" mode (default)
 -I number    Size of raw input buffer (default 2048)
 -f           Enable hardware flow control (default)
 -F           Disable hardware flow control
 -O number    Size of output buffer (default 2048)
 -s           Enable software flow control
 -S           Disable software flow control (default)
 -u unit      Set serial unit number (default 1)
#endif
*/
#include "externs.h"


unsigned
options(int argc, char *argv[])
{
	int			opt;
	int			numports = 0;
	unsigned	unit;
	static TTYINIT_USART devinit = {
		{
			0xFFFFF200,	// port
			0,			// port_shift
			1,			// intr
			115200,		// baud
			2048,		// isize
			2048,		// osize
			256,		// csize
			0,			// c_cflag
			0,			// c_iflag
			0,			// c_lflag
			0,			// c_oflag
			0,			// fifo
			59904000,	// pfclk
			16,			// div
			"/dev/ser"	// name
		},
	};

	/*
	 * Initialize the devinit to raw mode
	 */
	ttc(TTC_INIT_RAW, &devinit, 0);

	unit = 1;

	while (optind < argc) {
		/*
		 * Process dash options.
		 */
		while ((opt = getopt(argc, argv, IO_CHAR_SERIAL_OPTIONS "c:u:")) != -1) {
			switch (ttc(TTC_SET_OPTION, &devinit, opt)) {
			case 'c':
				devinit.tty.clk = strtoul(optarg, &optarg, 0);
				break;

			case 'u':
				unit = strtoul(optarg, NULL, 0);
				break;
			}
		}

		/*
		 * Process ports and interrupts.
		 */
		while (optind < argc && *(optarg = argv[optind]) != '-') {
			devinit.tty.port = strtoul(optarg, &optarg, 16);
			if (*optarg == ',') {
				devinit.tty.intr = strtoul(optarg + 1, &optarg, 0);
			}

			if (devinit.tty.port != 0 && devinit.tty.intr != -1) {
				create_device(&devinit, unit++);
				++numports;
			}
			++optind;
		}
	}

	if (numports == 0) {
		if (devinit.tty.port != 0 && devinit.tty.intr != -1) {
			create_device(&devinit, unit++);
			++numports;
		}
	}

	return numports;
}
