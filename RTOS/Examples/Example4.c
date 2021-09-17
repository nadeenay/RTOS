
#include "UARTs.h"

/* Tivaware includes */
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"


/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"

/* Shared value  */
uint8_t z=0,l=0;

#define FIRST_STACK 128

void Task1()
{
    if(z!=l)
    {
    UART0_SendString(" there is a shared memory problem ");
    UART0_SendString("\r\n");
    }
    vTaskDelay(47);
}

void Task2()
{
    z++;

    /* if task 1 interrupt task2 in this point there will be a problem shared memory */

    l++;

}

void Example4()
{

    /* Start RTOS Code */
    TaskHandle_t  First_handle;
    TaskHandle_t  Second_handle;

    /* Create tasks */
    xTaskCreate(Task1, "First",FIRST_STACK, NULL,2, &First_handle);
    xTaskCreate(Task2, "Second",FIRST_STACK, NULL,1, &Second_handle);

    /* StartScheduler */
    vTaskStartScheduler();

}




