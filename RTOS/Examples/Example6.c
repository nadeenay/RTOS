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
#include "semphr.h"

#define FIRST_STACK 100

/* Create semphr handles */
SemaphoreHandle_t sem1;


void Task1_6()
{
    uint8_t Client_Num=0;
    /*create semphr */
    sem1= xSemaphoreCreateBinary();
    xSemaphoreTake( sem1,(TickType_t )0);
    while(1)
    {
        /* try to take the semphr again but can't until the butten is pressed */

        if(xSemaphoreTake( sem1,(TickType_t )0)==pdTRUE)
        {
            UART0_SendString("Client num :");
            /* Take care this function will print till 9 only !! */
            UART0_SendInt(++Client_Num);
            UART0_SendString("Has served");
            UART0_SendString("\r\n");
        }
        /* Delay for 100 ms */
         vTaskDelay(100);

    }


}

void Task2_6()
{

     uint32_t data,pre=1;

     while(1)
     {
         data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);

         if((data & GPIO_PIN_4)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
         {
             xSemaphoreGive( sem1 );
         }

         pre=data;


     }




}
void Example6()
{
    /* Configuration  the RGB pins */

       /*Initialize the Clock System */
       SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

        /*Enable the Clock for PortF */
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

       /* Unlock Pins of the SW */
       GPIOUnlockPin(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);
       /* Define the SW as Input pins */
       GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);

       /* Start RTOS Code */
           TaskHandle_t  First_handle;
           TaskHandle_t  Second_handle;

       /*Creat the task */
        xTaskCreate(Task1_6, "Task1_6",FIRST_STACK, NULL,2, &First_handle);
        xTaskCreate(Task2_6, "Task2_6",FIRST_STACK, NULL,1, &Second_handle);

        /* StartScheduler */
        vTaskStartScheduler();


}




