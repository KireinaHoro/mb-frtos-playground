/*
 * Empty C++ Application
 */

#include "tasks.h"

XUartNs550 UartNs550;
XGpio GpioLed;

#define mainPRINT_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainBUTTON_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
#define mainLED_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

// override weak default in port_exceptions.c
void vApplicationExceptionRegisterDump( xPortRegisterDump *xRegisterDump ) {
	xil_printf("\r\nException: PC=0x%lx ESR=0x%lx EAR=0x%lx\r\n", xRegisterDump->ulPC, xRegisterDump->ulESR, xRegisterDump->ulEAR);
	xil_printf("           cause: %s\r\n", xRegisterDump->pcExceptionCause);
	if (xRegisterDump->pcCurrentTaskName) {
		xil_printf("           in task \"%s\"\r\n", xRegisterDump->pcCurrentTaskName);
	} else {
		xil_printf("           <no task name available>\r\n");
	}
}

#define CREATE_TASK_WITH_ARG(x, priority, arg) \
	/*xil_printf("Create " #x " task\r\n");*/ \
	TaskHandle_t x##Task; \
	if (xTaskCreate(x, \
			#x, \
			configMINIMAL_STACK_SIZE * 2U, \
			(void*)(arg), \
			main##priority##_TASK_PRIORITY, \
			&x##Task) != pdPASS) { \
		/*xil_printf("Failed to create " #x " task\r\n");*/ \
		return XST_FAILURE; \
	}
#define CREATE_TASK(x, priority) \
	CREATE_TASK_WITH_ARG(x, priority, NULL)

int main() {
	int Status;
	Status = setup_devices();
	if (Status != XST_SUCCESS) {
		return Status;
	}

	xil_printf("\r\n===== FreeRTOS app demo starting =====\r\n");

	CREATE_TASK(PrintHello, PRINT)
	CREATE_TASK(BlinkLed, LED)
	CREATE_TASK_WITH_ARG(PushButton, BUTTON, &GpioLed)

	if (ShellSpawn(&UartNs550) != pdPASS) {
		return XST_FAILURE;
	}

	xil_printf("Starting scheduler...\r\n");
	vTaskStartScheduler();

	// we should not reach here
	while (true);

	return 0;
}
