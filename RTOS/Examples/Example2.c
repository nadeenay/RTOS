#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"
#define FIRST_STACK        128

void Task1_2(void *pvParameters)
{
    TickType_t LastWake;
    LastWake=xTaskGetTickCount();
    uint8_t state= 0x04;
    while(1)
    {
        /*Turn the blue led*/
         GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, state);

         state^=0xFF;

         /* Delay */
         vTaskDelayUntil(&LastWake,1000);


    }

}

void Task2_2(void *pvParameters)
{
    while(1)
    {    /*Turn on the red led*/
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0x02);

        /*Delay */
        vTaskDelay(100);

        /*Turn off the red led */
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0x00);


        /*Delay */
        vTaskDelay(100);



    }

}
void Example2(){
     /*Initialize the Clock System */
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

     /*Enable the Clock for PortF */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /*Make Pin 1,2 as Output Pin */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2 | GPIO_PIN_1);

    /* Start RTOS Code */
    TaskHandle_t  First_handle;
    TaskHandle_t  Second_handle;

    /*Creat the tasks */
    xTaskCreate(Task1_2, "First",FIRST_STACK, NULL,1, &First_handle);
    xTaskCreate(Task2_2, "Second",FIRST_STACK, NULL,2, &Second_handle);


    /* StartScheduler */
    vTaskStartScheduler();

}









