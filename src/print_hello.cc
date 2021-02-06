/*
 * print_hello.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

#define mainPRINT_FREQ_MS pdMS_TO_TICKS(60000)
void PrintHello(void *pvParameters) {
	TickType_t xNextWakeTime;
	int counter = 0;

	xNextWakeTime = xTaskGetTickCount();

	while (true) {
		xil_printf(">>> %d minutes since startup\r\n", counter++);

		vTaskDelayUntil(&xNextWakeTime, mainPRINT_FREQ_MS);
	}
}


