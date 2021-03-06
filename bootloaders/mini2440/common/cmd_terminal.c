/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <devices.h>
#include <serial.h>

#if defined(CONFIG_CMD_TERMINAL)

static void init_serial_by_name(char *name)
{
	struct serial_device *ser;
	for (ser = serial_devices; ser; ser = ser->next) {
		if (!strcmp(ser->name, name)) {
			ser->init();
			return;
		}
	}
}

int do_terminal(cmd_tbl_t * cmd, int flag, int argc, char *argv[])
{
	int i, l;
	int last_tilde = 0;
	device_t *dev = NULL;

	if (argc < 1)
		return -1;

	/* Scan for selected output/input device */
	for (i = 1; i <= ListNumItems (devlist); i++) {
		device_t *tmp = ListGetPtrToItem (devlist, i);
		if (!strcmp(tmp->name, argv[1])) {
			dev = tmp;
			break;
		}
	}
	if (!dev)
		return -1;

#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4) || \
    defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3) || \
    defined(CONFIG_ARCH_GTA01B_v4)
	if (!strcmp(dev->name, "serial") ||
	    !strcmp(dev->name, "s3ser0"))
		neo1973_gta01_serial0_gsm(1);
#endif

	init_serial_by_name(dev->name);
	//serial_reinit_all();
	printf("Entering terminal mode for port %s\n", dev->name);
	puts("Use '~.' to leave the terminal and get back to u-boot\n");

	while (1) {
		int c;

		/* read from console and display on serial port */
		if (stdio_devices[0]->tstc()) {
			c = stdio_devices[0]->getc();
			if (last_tilde == 1) {
				if (c == '.') {
					putc(c);
					putc('\n');
					break;
				} else {
					last_tilde = 0;
					/* write the delayed tilde */
					dev->putc('~');
					/* fall-through to print current
					 * character */
				}
			}
			if (c == '~') {
				last_tilde = 1;
				puts("[u-boot]");
				putc(c);
			}
			dev->putc(c);
		}

		/* read from serial port and display on console */
		if (dev->tstc()) {
			c = dev->getc();
			putc(c);
		}
	}
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4) || \
    defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3) || \
    defined(CONFIG_ARCH_GTA01B_v4)
	if (!strcmp(dev->name, "serial") ||
	    !strcmp(dev->name, "s3ser0"))
		neo1973_gta01_serial0_gsm(0);
#endif

	return 0;
}


/***************************************************/

U_BOOT_CMD(
	terminal,	3,	1,	do_terminal,
	"terminal - start terminal emulator\n",
	""
);

#endif /* CONFIG_CMD_TERMINAL */
