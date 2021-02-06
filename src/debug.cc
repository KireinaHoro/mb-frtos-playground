/*
 * debug.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

#define RETURN "\r\n"

void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		xil_printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			xil_printf(" ");
			if ((i+1) % 16 == 0) {
				xil_printf("|  %s " RETURN, ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					xil_printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					xil_printf("   ");
				}
				xil_printf("|  %s " RETURN, ascii);
			}
		}
	}
}


