/*
 * blink_led.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

#define mainLED_FREQ_MS pdMS_TO_TICKS(50)
void BlinkLed(void *pvParameters) {
	TickType_t xNextWakeTime;
	u8 led = 0b00000001;
	u8 direction = 0; // left

	xNextWakeTime = xTaskGetTickCount();

	while (true) {
		XGpio_DiscreteWrite(&GpioLed, LED_CHANNEL, led);

		// update the LED array
		if (led == 0b10000000 && direction == 0) {
			direction = 1; // right
		} else if (led == 0b00000001 && direction == 1){
			direction = 0;
		}
		if (direction) {
			led >>= 1;
		} else {
			led <<= 1;
		}

		//xil_printf("LED vector: 0x%x\r\n", led);

		vTaskDelayUntil(&xNextWakeTime, mainLED_FREQ_MS);
	}
}



