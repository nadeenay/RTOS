
#include "UARTs.h"

/*Tivaware includes */
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"

/*This include for calling GPIO_Pin_Config()*/
#include "driverlib/pin_map.h"

/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#define SWITCHTASKSTACKSIZE        256

extern void UART0_Send_String(char* str);



EventGroupHandle_t Group1;

void Task12(void *para)
{
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_1);
    while(1)
    {
         static uint32_t state =0;
         xEventGroupWaitBits(Group1,0x01 ,pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
         GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,state);
         state ^= GPIO_PIN_1;
    }
}


void Button2(void * para)
{
    uint32_t data = 0;
    uint8_t pre= 1 ; //to work on falling edge
    while(1)
    {
        data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);

        if((data & GPIO_PIN_0)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
        {
            xEventGroupSetBits(Group1,0x01);
           //GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_2);
        }
        pre = (data & GPIO_PIN_0);
        vTaskDelay(10);
    }

}
void SWs() {
      /*UART configuration */
      UART0_Config();

      /*Enable clock for portF*/
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
      /* Unlock Pins of the SW */
      GPIOUnlockPin(GPIO_PORTF_BASE,GPIO_PIN_0);
      /* Define the SW as Input pins */
      GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);

      /*Define LEDs
       *  as OUTPUTs */

      GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1 |GPIO_PIN_2|GPIO_PIN_3);

    TaskHandle_t  Second_handle,Third_handle;
    Group1 = xEventGroupCreate();

    xTaskCreate(Button2, "Button2",SWITCHTASKSTACKSIZE, NULL,2, &Second_handle);
    xTaskCreate(Task12, "Task1",SWITCHTASKSTACKSIZE, NULL,3, &Third_handle);

    vTaskStartScheduler();

}




