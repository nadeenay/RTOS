
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

#define Task_Stack_Size        128

uint8_t speed1 = 1;


void button1(void * para)
{
    uint32_t data = 0;
    uint8_t pre= 0x01 ; //to work on falling edge
    while(1)
    {
        data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);

        if((data & GPIO_PIN_4)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
        {
            speed1 += 2 ;
            speed1 = (speed1 > 10) ? 1 :speed1;
        }
        pre = data;
        vTaskDelay(25); //ever 25 tick, as 25 ms
    }

}
void led1(void * para)
{
    uint8_t state = GPIO_PIN_1;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {
        vTaskDelayUntil( &xLastWakeTime, 100*speed1);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,state);
        state ^= 0xFF;
    }
}
void TaskState2(void *para)
{
    /* Get the tasks Handle */
        TaskHandle_t LED1 =xTaskGetHandle("LED1");
        TaskHandle_t Button1 =xTaskGetHandle("Button1");
        TaskHandle_t TaskState2 =xTaskGetHandle("TaskState2");


        TaskStatus_t T1_info, T2_info, T3_info;
        TaskStatus_t *Task_Info_Array;
        UBaseType_t number,i;
        uint32_t TotalRunTime;

        /* Get the number of tasks in the system */
        number =uxTaskGetNumberOfTasks();

        Task_Info_Array = pvPortMalloc( number * sizeof( TaskStatus_t ) );

        /* Get the task's info in Task_Info_Array */
        uxTaskGetSystemState(Task_Info_Array,number,&TotalRunTime );


        UART0_SendString("Number of tasks = ");
        UART0_SendInt(number);
        UART0_SendString("\r\n");
    for(i = 0 ; i < number ; i ++ )   /* To print tasks names at the beginning */
    {
        UART0_SendString(Task_Info_Array[i].pcTaskName);
        UART0_SendString("\r\n");
    }

    while(1)
    {
        /*Get the task's info */
         vTaskGetInfo(LED1,&T1_info, pdTRUE,eInvalid );
         vTaskGetInfo(Button1,&T2_info, pdTRUE,eInvalid );
         vTaskGetInfo(TaskState2,&T3_info, pdTRUE,eInvalid );

        UART0_SendString(T1_info.pcTaskName);
        UART0_SendString("\r\n");
        UART0_SendInt(T1_info.eCurrentState);
        UART0_SendString("\r\n");
        UART0_SendString(T2_info.pcTaskName);
        UART0_SendString("\r\n");
        UART0_SendInt(T2_info.eCurrentState);
        UART0_SendString("\r\n");
        UART0_SendInt(T3_info.eCurrentState);
        UART0_SendString("\r\n");
        UART0_SendInt(T3_info.eCurrentState);
        UART0_SendString("\r\n");
        vTaskDelay(1000); //1 second
    }
}

 void Demo2(){

     /*Initialize the Clock System */
     SysCtlClockSet(SYSCTL_SYSDIV_1|SYSCTL_USE_OSC|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

     /*Enable clock for portF*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    /* Unlock Pins of the SW */
    GPIOUnlockPin(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);
    /* Define the SW as Input pins */
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);

    /*Define Leds as OUTPUTs */
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1 | GPIO_PIN_2|GPIO_PIN_3);

    /* Uart Config */
    UART0_Config();


     /*Create handles and tasks */
    TaskHandle_t  First_handle,Second_handle,Third_handle;
    xTaskCreate(led1, "LED1",Task_Stack_Size, NULL,1, &First_handle);
    xTaskCreate(button1, "Button1",Task_Stack_Size, NULL,2, &Second_handle);
    xTaskCreate(TaskState2, "TaskState2",Task_Stack_Size,NULL,3, &Third_handle);


   /*Start schudler */
    vTaskStartScheduler();

}







