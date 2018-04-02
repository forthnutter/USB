/* HID Listen, http://www.pjrc.com/teensy/hid_listen.html
 * Listens (and prints) all communication received from a USB HID device,
 * which is useful for view debug messages from the Teensy USB Board.
 * Copyright 2008, PJRC.COM, LLC
 *
 * You may redistribute this program and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 */




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rawhid.h"


static void delay_ms(unsigned int msec);


int main(void)
{
	char buf[64], *in, *out;
	rawhid_t *hid;
	int num, count;

	printf("Waiting for device:");
	fflush(stdout);
	while (1) {
		hid = rawhid_open_only1(0, 0, 0xFF31, 0x0074);
		if (hid == NULL) {
			printf(".");
			fflush(stdout);
			delay_ms(1000);
			continue;
		}
		printf("\nListening:\n");
		while (1) {
			num = rawhid_read(hid, buf, sizeof(buf), 200);
			if (num < 0) break;
			if (num == 0) continue;
			in = out = buf;
			for (count=0; count<num; count++) {
				if (*in) {
					*out++ = *in;
				}
				in++;
			}
			count = out - buf;
			//printf("read %d bytes, %d actual\n", num, count);
			if (count) {
				num = fwrite(buf, 1, count, stdout);
				fflush(stdout);
			}
		}
		rawhid_close(hid);
		printf("\nDevice disconnected.\nWaiting for new device:");
	}
	return 0;
}






#if (defined(WIN32) || defined(WINDOWS) || defined(__WINDOWS__)) 
#include <windows.h>
static void delay_ms(unsigned int msec)
{
	Sleep(msec);
}
#else
#include <unistd.h>
static void delay_ms(unsigned int msec)
{
	usleep(msec * 1000);
}
#endif
