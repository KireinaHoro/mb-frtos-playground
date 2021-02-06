/*
 * mem_check.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

#define DDR_BASE 0x100000100UL
#define CHECK_SIZE 0x100000
#define PATTERN_1 0xaaaaaaaaul
#define PATTERN_2 0x33333333ul
void MemCheck(void *pvParameters) {
	xil_printf("sizeof(UINTPTR) = %d\r\n", sizeof(UINTPTR));
	u64 data;
	for (int i = 0; i < CHECK_SIZE; ++i) {
		UINTPTR ptr = DDR_BASE + i * sizeof(u64);
		//xil_printf("Write 0x%lx to 0x%lx\r\n", PATTERN_1, ptr);
		Xil_Out64(ptr, PATTERN_1);
		data = Xil_In64(ptr);
		if (data != PATTERN_1) {
			xil_printf("0x%lx mismatch: expected 0x%lx, got 0x%lx\r\n", ptr, PATTERN_1, data);
		}
		//xil_printf("Read 0x%lx from 0x%lx\r\n", data, ptr);
		//xil_printf("Write 0x%lx to 0x%lx\r\n", PATTERN_2, ptr);
		Xil_Out64(ptr, PATTERN_2);
		data = Xil_In64(ptr);
		if (data != PATTERN_2) {
			xil_printf("0x%lx mismatch: expected 0x%lx, got 0x%lx\r\n", ptr, PATTERN_2, data);
		}
		//xil_printf("Read 0x%lx from 0x%lx\r\n", data, ptr);
	}
}



