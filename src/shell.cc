/*
 * shell.cc
 *
 *  Created on: 2021/02/06
 *      Author: jsteward
 */

#include "tasks.h"

#include <queue.h>

//#define READLINE_DEBUG
#define READLINE_ECHO

#define mainREADLINE_PRIO (tskIDLE_PRIORITY + 4)
#define mainPROCESS_PRIO (tskIDLE_PRIORITY + 3)
#define mainQUEUE_LENGTH 8
#define mainBUFFER_LENGTH 4096

#define UART_IRPT_INTR XPAR_MICROBLAZE_0_AXI_INTC_AXI_UART16550_0_IP2INTC_IRPT_INTR

static QueueHandle_t xQueue = NULL;

typedef struct {
	XUartNs550 *UartNs550Ptr;
	TaskHandle_t HandlingTask;
	u32 TotalSentCount;
	u32 TotalReceivedCount;
	u32 TotalErrorCount;
} ReadlineData_t;

static void vUartHandler(void *CallBackRef, u32 Event, unsigned int EventData) {
	u8 Errors;
	ReadlineData_t *data = (ReadlineData_t *)CallBackRef;

	switch (Event) {
	/*
	 * All of the data has been sent.
	 */
	case XUN_EVENT_SENT_DATA:
		data->TotalSentCount = EventData;
		break;
	/*
	 * All of the data has been received.
	 */
	case XUN_EVENT_RECV_DATA:
	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	case XUN_EVENT_RECV_TIMEOUT:
		data->TotalReceivedCount = EventData;

		// wake the read handler
		BaseType_t xHigherPriorityTaskWoken;
		vTaskNotifyGiveFromISR(data->HandlingTask, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		break;
	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	case XUN_EVENT_RECV_ERROR:
		data->TotalReceivedCount = EventData;
		data->TotalErrorCount++;
		Errors = XUartNs550_GetLastErrors(data->UartNs550Ptr);
		break;
	default:
		configASSERT(pdFALSE);
	}
}

static void ShellReadline(void *pvParameters) {
	int Status;
	ReadlineData_t data = {0};

	XUartNs550 *UartNs550Ptr = (XUartNs550 *)pvParameters;
	data.UartNs550Ptr = UartNs550Ptr;
	data.HandlingTask = xTaskGetCurrentTaskHandle();

	/*
	 * Setup the handlers for the UART that will be called from the
	 * interrupt context when data has been sent and received, specify a
	 * pointer to the UART driver instance as the callback reference so
	 * the handlers are able to access the instance data.
	 */
	XUartNs550_SetHandler(UartNs550Ptr, vUartHandler, &data);

	// enable UART interrupt
	Status = xPortInstallInterruptHandler( UART_IRPT_INTR, (XInterruptHandler)&XUartNs550_InterruptHandler, UartNs550Ptr );
	configASSERT(Status == pdPASS);
	vPortEnableInterrupt( UART_IRPT_INTR );

	/*
	 * Enable the interrupt of the UART so interrupts will occur, setup
	 * a local loopback so data that is sent will be received, and keep the
	 * FIFOs enabled.
	 */
	int Options = XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(UartNs550Ptr, Options);

	uint32_t ulNotifiedValue;
	const TickType_t xBlockTime = pdMS_TO_TICKS(1000);

	char *buffer = (char *)pvPortMalloc(mainBUFFER_LENGTH + 1); // zero terminated
	u32 recvHead = 0;
	u32 leftoverStart, leftoverCount;
	// start the readline loop
	while (true) {
		leftoverStart = 0;
		leftoverCount = 0;
		while (recvHead < mainBUFFER_LENGTH) {
			// launch the receive
			XUartNs550_Recv(UartNs550Ptr, (u8 *)(buffer + recvHead), mainBUFFER_LENGTH - recvHead);
			while (!(ulNotifiedValue = ulTaskNotifyTake(pdTRUE, xBlockTime))) {
#ifdef READLINE_DEBUG
				xil_printf("waiting for uart interrupt...\r\n");
#endif
			}
#ifdef READLINE_DEBUG
			xil_printf("debug: received %d chars, %d errors. dumping buffer...\r\n", data.TotalReceivedCount, data.TotalErrorCount);
			hexdump(buffer, mainBUFFER_LENGTH + 1);
#endif
			for (u32 check = 0; check < data.TotalReceivedCount; ++check) {
				// Return is \r
				char readChar = buffer[recvHead + check];
				if (readChar == '\r') {
					// found newline, end current buffer
					leftoverStart = recvHead + check + 1;
					leftoverCount = data.TotalReceivedCount - check - 1;
#ifdef READLINE_DEBUG
					xil_printf("found return: leftoverStart=%d leftoverCount=%d\r\n", leftoverStart, leftoverCount);
#endif
					// stuff \r
					buffer[recvHead + check] = '\0';
#ifndef READLINE_ECHO
					break;
#else
					xil_printf("\r\n");
#endif
				}
#ifdef READLINE_ECHO
				else if (readChar == '\x7f') {
					// replace DEL with ^H to prevent unexpected results
					xil_printf("^H");
				} else {
					xil_printf("%c", readChar);
				}
#endif
			}
			recvHead += data.TotalReceivedCount;
			if (leftoverStart || leftoverCount) {
#ifdef READLINE_DEBUG
				xil_printf("found return, finishing buffer, leaving %d bytes behind\r\n", leftoverCount);
#endif
				break;
			} else {
#ifdef READLINE_DEBUG
				xil_printf("continuing, recvHead now at %d\r\n", recvHead);
#endif
			}
		}

		// migrate the left bytes and terminate the string
		char *newBuffer = (char *)pvPortMalloc(mainBUFFER_LENGTH + 1);
		if (leftoverCount) {
			memcpy(newBuffer, buffer + leftoverStart, leftoverCount);
			buffer[leftoverStart] = '\0';
		} else {
			buffer[recvHead] = '\0';
		}

		// submit the string
		while (xQueueSend(xQueue, &buffer, xBlockTime) != pdPASS) {
#ifdef READLINE_DEBUG
			xil_printf("waiting for queue send...\r\n");
#endif
		}
		buffer = newBuffer;
		recvHead = leftoverCount;
	}
}

static void ShellProcess(void *pvParameters) {
	char *recvStr;
	while (true) {
		xQueueReceive(xQueue, &recvStr, portMAX_DELAY);

		configASSERT(recvStr);
		xil_printf("received: \"%s\"\r\n", recvStr);

		vPortFree(recvStr);
	}
}

int ShellSpawn(XUartNs550 *UartNs550Ptr) {
	int Status;
	// create queue
	xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(char *));
	if (!xQueue) {
		return pdFAIL;
	}
	// pass the UART instance in
	Status = xTaskCreate(ShellReadline, "ShellReadline", configMINIMAL_STACK_SIZE * 2u, UartNs550Ptr, mainREADLINE_PRIO, NULL);
	if (Status != pdPASS) {
		return Status;
	}
	Status = xTaskCreate(ShellProcess, "ShellProcess", configMINIMAL_STACK_SIZE * 2u, NULL, mainPROCESS_PRIO, NULL);
	if (Status != pdPASS) {
		return Status;
	}

	return pdPASS;
}
