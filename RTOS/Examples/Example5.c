

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
#include "queue.h"

/* Some #defines */

#define RED   GPIO_PIN_1
#define BLUE  GPIO_PIN_2
#define GREEN GPIO_PIN_3


#define FIRST_STACK  128
/* Shared value  */
int x=0,y=0;
uint32_t color[]={RED,BLUE,GREEN};
char*colorS={"RED","BLUE","GREEN"};
/* Create Queue handles */
QueueHandle_t Q1;
QueueHandle_t Q2;


void Task1_5()
{
    uint32_t i=0,col=RED;
         char*colS="RED";
         while(1)
         {
          if(xQueueReceive(Q2,(void*)&colS,(TickType_t)0)==pdTRUE & xQueueReceive(Q1,(void*)&col,(TickType_t)0)==pdTRUE)
         {
        UART0_SendString(colS);
        UART0_SendString("\r\n");
         }
        GPIOPinWrite(GPIO_PORTF_BASE,col,col);
        vTaskDelay(50);
        GPIOPinWrite(GPIO_PORTF_BASE,col,0x00);
        vTaskDelay(50);
         }


}

void Task2_5()
{
    /* Create 2 Queues one for Sending strings and one for Sending  the integers */
     Q1=xQueueCreate(2 ,sizeof(uint32_t) );
     Q2=xQueueCreate(2 ,sizeof(char*));

     /* start the main task */
     uint32_t i=0,col,data,pre=1;
     char*colS;

     while(1)
     {
         data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);

         if((data & GPIO_PIN_4)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
         {
         col=color[i];
         colS=colorS[i++];

         /* Send to the Queue */
         xQueueSend(Q1,(void*)&col,(TickType_t)100);
         xQueueSend(Q2,(void*)&colS,(TickType_t)100);
         i%=3;
         }

         pre=data;
         /* Delay for 100 ms */
         vTaskDelay(100);

     }




}
void Example5()
{

    /*Initialize the Clock System */
    SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    /* Configuration  the RGB pins */

    /*Initialize the Clock System */
       SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

        /*Enable the Clock for PortF */
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

       /* Unlock Pins of the SW */
       GPIOUnlockPin(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);
       /* Define the SW as Input pins */
       GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);

       /*Make Pin 1,2,3 as Output Pin */
       GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3);

       /* Start RTOS Code */
           TaskHandle_t  First_handle;
           TaskHandle_t  Second_handle;

       /*Creat the task */
        xTaskCreate(Task1_5, "Task1_5",FIRST_STACK, NULL,1, &First_handle);
        xTaskCreate(Task2_5, "Task2_5",FIRST_STACK, NULL,2, &Second_handle);

        /* StartScheduler */
        vTaskStartScheduler();


}




