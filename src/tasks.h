/*
 * tasks.h
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#ifndef SRC_TASKS_H_
#define SRC_TASKS_H_

#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>

#include "xparameters.h"
#include "xuartns550.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"

void MemCheck(void *pvParameters);
void BlinkLed(void *pvParameters);
void PrintHello(void *pvParameters);
void PushButton(void *pvParameters);

int ShellSpawn(XUartNs550 *UartNs550Ptr);

int setup_devices();

void hexdump(const void* data, size_t size);

extern XUartNs550 UartNs550;
extern XGpio GpioLed;

// to board UART
#define UART_DEVICE_ID XPAR_UARTNS550_0_DEVICE_ID

// to board LED (o, channel 2) and push buttons (i, channel 1)
#define GPIO_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID

#define PUSH_BUTTON_CHANNEL 1
#define LED_CHANNEL 2

#endif /* SRC_TASKS_H_ */
