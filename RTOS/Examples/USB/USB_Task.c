#include "USB.h"
#include "usb_serial_structs.h"

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
#include "semphr.h"

#define Task_Stack_Size        128

/* Create Queue handles */
QueueHandle_t Q;


/* Create semaphore */
SemaphoreHandle_t SEM_USB;

/*Global flag indicating that a USB configuration has been set*/
volatile bool g_bUSBConfigured = false;

void RGB_USB(void * para)
{
     /* this task have a high priority so create Semaphore here */
     SEM_USB= xSemaphoreCreateBinary();

     /* Create the Q communicate between USB_task and  RGB task*/
     Q=xQueueCreate(2 ,sizeof(uint8_t) );


     /*buffer to receive character from the queue */
    uint8_t Rx_buffer;
    while(1)
    {

    if(xQueueReceive(Q,(void*)&Rx_buffer,(TickType_t)portMAX_DELAY)==pdTRUE)
    {

        switch(Rx_buffer)
        {

        case 'r':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_1);
            break;

        case 'b':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_2);
            break;

        case 'g':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_3);
            break;


        }
    }
    }
}




void USB0(){



    /*80 MHz Clock*/
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5);
    /* USB configuration */
    USB_serialInit();

    /*Enable clock for portF*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

     /*Define LEDs
       *  as OUTPUTs */
     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1 |GPIO_PIN_2|GPIO_PIN_3);

    /* Create Handles */
    TaskHandle_t  RGB_handle;

    /* Create 1 Tasks */
    xTaskCreate( RGB_USB, " RGB_USB",Task_Stack_Size, NULL,1, &RGB_handle);

    /* Start Scheduler */
    vTaskStartScheduler();

}




