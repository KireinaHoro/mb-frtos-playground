/*
 * push_button.cc
 *
 *  Created on: 2021/02/19
 *      Author: jsteward
 */

#include "tasks.h"

#define GPIO_IRPT_INTR XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_0_IP2INTC_IRPT_INTR

typedef struct {
	XGpio *GpioPtr;
	TaskHandle_t HandlingTask;
	int IntrMask;
} PushButtonData_t;

static void vPushButtonHandler(void *CallBackRef) {
	PushButtonData_t *data = (PushButtonData_t *)CallBackRef;

	// wake the read handler
	BaseType_t xHigherPriorityTaskWoken;
	vTaskNotifyGiveFromISR(data->HandlingTask, &xHigherPriorityTaskWoken);

	XGpio_InterruptClear(data->GpioPtr, data->IntrMask);

	// yield from ISR
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void PushButton(void *pvParameters) {
	int Status;
	PushButtonData_t data = {0};

	XGpio *GpioPtr = (XGpio *)pvParameters;
	data.GpioPtr = GpioPtr;
	data.HandlingTask = xTaskGetCurrentTaskHandle();
	data.IntrMask = PUSH_BUTTON_CHANNEL;

	// install handler
	Status = xPortInstallInterruptHandler(GPIO_IRPT_INTR, (XInterruptHandler)&vPushButtonHandler, &data);
	configASSERT(Status == pdPASS);
	vPortEnableInterrupt(GPIO_IRPT_INTR);

	// enable interrupt on GPIO controller
	XGpio_InterruptEnable(data.GpioPtr, data.IntrMask);
	XGpio_InterruptGlobalEnable(data.GpioPtr);

	// handle task notifications
	uint32_t ulNotifiedValue;
	uint32_t oldValue = XGpio_DiscreteRead(data.GpioPtr, PUSH_BUTTON_CHANNEL);
	const TickType_t xBlockTime = pdMS_TO_TICKS(10000); // 10s
	while (true) {
		while (!(ulNotifiedValue = ulTaskNotifyTake(pdTRUE, xBlockTime))) {
			//xil_printf("waiting for GPIO interrupt...\r\n");
		}
		//xil_printf("interrupt!\r\n");
		uint32_t newValue = XGpio_DiscreteRead(data.GpioPtr, PUSH_BUTTON_CHANNEL);
		uint32_t changed = newValue ^ oldValue;
		if (changed & 0x1) xil_printf("C%c ", newValue & 0x1 ? '+' : '-');
		if (changed & 0x2) xil_printf("N%c ", newValue & 0x2 ? '+' : '-');
		if (changed & 0x4) xil_printf("W%c ", newValue & 0x4 ? '+' : '-');
		if (changed & 0x8) xil_printf("S%c ", newValue & 0x8 ? '+' : '-');
		if (changed & 0x10) xil_printf("E%c ", newValue & 0x10 ? '+' : '-');
		if (changed) xil_printf("\r\n");
		oldValue = newValue;
	}
}

