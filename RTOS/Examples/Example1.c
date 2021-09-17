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

void delay(void ) //not best practice
{
  int i = 1000000;
    while (i > 0)
    {
      i--;
    }
}

void First_Task(void *pvParameters)
{
    while(1)
    {    /*Turn the red led*/
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, GPIO_PIN_1);


        /*Delay */
        SysCtlDelay(2000000);

        /*Turn the blue*/
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, GPIO_PIN_2);

        /*Delay */
        SysCtlDelay(2000000);

        /*Turn the green */

        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, GPIO_PIN_3);

        /*Delay */
        SysCtlDelay(2000000);

    }

}
void Example1(){
     /*Initialize the Clock System */
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

     /*Enable the Clock for PortF */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /*Make Pin 1,2,3 as Output Pin */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3);

    /* Start RTOS Code */
    TaskHandle_t  First_handle;

    /*Creat the task */
    xTaskCreate(First_Task, "First",FIRST_STACK, NULL,1, &First_handle);


    /* StartScheduler */
    vTaskStartScheduler();


}





