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

#define TASK_STACK 100

/* Create semphr handles */
SemaphoreHandle_t sem3;
/* Shared resources */
uint8_t O_Count=0;
uint8_t H_Count=0;


void Task1_7()
{


    /*create semphr */
    sem3= xSemaphoreCreateBinary();

    while(1)
    {

        if(O_Count>1 & H_Count>2)
        {
            xSemaphoreTake( sem3,(TickType_t )0);
            O_Count--;
            O_Count--;
            UART0_SendString("Gain one H2O ! ");
            UART0_SendString("\r\n");
            xSemaphoreGive( sem3);

        }

        vTaskDelay(100); /*delay for 100 ms*/

    }


}

void Task2_7()
{

     uint32_t data,pre=1;

     while(1)
     {
         data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);

         /*  SW 1 is pressed */
         if((data & GPIO_PIN_4)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
         {
             xSemaphoreTake( sem3,(TickType_t )0);
             UART0_SendString("Gain one Oxygen ! O_Count = ");
             /* Take care this function will print till 9 only !! */
             UART0_SendInt(++O_Count);
             UART0_SendString("\r\n");
             xSemaphoreGive( sem3);


         }

         pre=data;
         vTaskDelay(25); /*delay for 100 ms*/
     }




}


void Task3_7()
{
         uint32_t data,pre=1;

         while(1)
         {
             data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);

             /*  SW 1 is pressed */
             if((data & GPIO_PIN_0)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
             {
                 xSemaphoreTake( sem3,(TickType_t )0);
                 UART0_SendString("Gain one hydrogen ! H_Count = ");
                 /* Take care this function will print till 9 only !! */
                 UART0_SendInt(++H_Count);
                 UART0_SendString("\r\n");
                 xSemaphoreGive( sem3);


             }

             pre=data;
         }
}
void Example7()
{


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
           TaskHandle_t  Thired_handle;


       /*Creat the task */
        xTaskCreate(Task1_7, "Task1_7",TASK_STACK, NULL,3, &First_handle);
        xTaskCreate(Task2_7, "Task2_7",TASK_STACK, NULL,2, &Second_handle);
        xTaskCreate(Task3_7, "Task3_7",TASK_STACK, NULL,1, &Thired_handle);


        /* StartScheduler */
        vTaskStartScheduler();


}




