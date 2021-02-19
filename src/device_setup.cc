/*
 * device_setup.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

static int setup_cache() {
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	// xil_printf("Caches enabled.\r\n");
	return XST_SUCCESS;
}

static int setup_uart() {
	int Status;
	// initialize UART
	Status = XUartNs550_Initialize(&UartNs550, UART_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	Status = XUartNs550_SelfTest(&UartNs550);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUartNs550Format Format = {
			.BaudRate = 115200,
			.DataBits = XUN_FORMAT_8_BITS,
			.Parity = XUN_FORMAT_NO_PARITY,
			.StopBits = XUN_FORMAT_1_STOP_BIT,
	};

	Status = XUartNs550_SetDataFormat(&UartNs550, &Format);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	xil_printf("\r\nUART started.\r\n");
	return Status;
}

static int setup_gpio() {
	int Status;

	// initialize GPIO
	Status = XGpio_Initialize(&GpioLed, GPIO_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	/* Set the direction for all signals as inputs except the LED output */
	//u32 led_mask = 0xff;
	//XGpio_SetDataDirection(&GpioLed, LED_CHANNEL, ~led_mask);

	xil_printf("GPIO started.\r\n");
	return Status;
}

#define RUN_SETUP(x) { \
	Status = setup_##x(); \
	if (Status != XST_SUCCESS) \
		return Status; \
}
int setup_devices() {
	int Status;

	RUN_SETUP(cache)
	RUN_SETUP(uart)
	RUN_SETUP(gpio)

	return XST_SUCCESS;
}
