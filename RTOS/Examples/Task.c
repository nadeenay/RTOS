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
#include "semphr.h"

#define Task_Stack_Size        128


/* global variable but it's bad practice */
bool IS_Send=false;
uint8_t Rec_Char;   /* Char received */
SemaphoreHandle_t sem2;

/* public Functions */
void UART0_Send_String(char* str)
{
    uint32_t i=0;
    while(str[i]!='\0')
    UARTCharPut(UART0_BASE,str[i++]);

    UARTCharPut(UART0_BASE,'\r');
    UARTCharPut(UART0_BASE,'\n');

}

/* this function for numbers form 0-->9 only */
void UART0_Send_Int(uint8_t i)
{
    UARTCharPut(UART0_BASE,i+48);
    UARTCharPut(UART0_BASE,'\r');
    UARTCharPut(UART0_BASE,'\n');
}

void UART_Task(void * para)
{

    while(1)
    {
    /* this task have a high priority so create the mutex here */
        sem2= xSemaphoreCreateBinary();

    /* check if there is a value to receive */
    if(UARTCharsAvail(UART0_BASE)) /*There is an Char in the FIFO*/
        {
          xSemaphoreTake( sem2,(TickType_t )0);
           uint8_t Rec_Char =(uint8_t)(UARTCharGetNonBlocking(UART0_BASE));
           UARTCharPut(UART0_BASE,Rec_Char);
           UARTCharPut(UART0_BASE,'\r');
           UARTCharPut(UART0_BASE,'\n');
           IS_Send=true;
           xSemaphoreGive( sem2);
        }
    vTaskDelay(100); /* Delay for 100ms */

    }


}


void RGB(void * para)
{
    while(1)
    {
    if(IS_Send) /*There is an Char in the FIFO*/
    {
        /* We can put here a mutex because of the shred recourse IS_Send */
        xSemaphoreTake( sem2,(TickType_t )0);
        switch(Rec_Char)
        {
        case 'r':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);

        case 'b':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

        case 'g':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
        }
        IS_Send=false;
        xSemaphoreGive( sem2);

    }

    }


}


void TaskState1(void *para)
{
    /* Get the tasks Handle */
    TaskHandle_t UART_Task_handle =xTaskGetHandle("UART_Task");
    TaskHandle_t RGB_handle =xTaskGetHandle("RGB");
    TaskHandle_t TaskState1_handle =xTaskGetHandle("TaskState1");


    TaskStatus_t T1_info, T2_info, T3_info;
    TaskStatus_t *Task_Info_Array;
    UBaseType_t number,i;
    uint32_t TotalRunTime;

    /* Get the number of tasks in the system */
    number =uxTaskGetNumberOfTasks();

    Task_Info_Array = pvPortMalloc( number * sizeof( TaskStatus_t ) );

    /* Get the task's info in Task_Info_Array */
    uxTaskGetSystemState(Task_Info_Array,number,&TotalRunTime );

    UART0_Send_String("Number of tasks = ");
    UART0_Send_Int(number);
    UARTCharPut(UART0_BASE,'\r');
    UARTCharPut(UART0_BASE,'\n');

    for(i = 0 ; i < number ; i ++ )   /* To print tasks names at the beginning */
    {
        UART0_Send_String(Task_Info_Array[i].pcTaskName);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

    }

    while(1)
    {
       /*Get the task's info */
        vTaskGetInfo(UART_Task_handle,&T1_info, pdTRUE,eInvalid );
        vTaskGetInfo(RGB_handle,&T2_info, pdTRUE,eInvalid );
        vTaskGetInfo(TaskState1_handle,&T3_info, pdTRUE,eInvalid );

        UART0_Send_String(T1_info.pcTaskName);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        UART0_Send_Int(T1_info.eCurrentState);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        UART0_Send_String(T2_info.pcTaskName);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        UART0_Send_Int(T2_info.eCurrentState);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        UART0_Send_String(T3_info.pcTaskName);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        UART0_Send_Int(T3_info.eCurrentState);
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        vTaskDelay(1000);

    }
}

 void Task(){


    UART0_Config();


    /* Create Handles */
    TaskHandle_t  UART_Task_handle,RGB_handle,TaskState1_handle;

    /* Create 3 Tasks */
    xTaskCreate(RGB, "RGB",Task_Stack_Size, NULL,1, &RGB_handle);
    xTaskCreate(UART_Task, "UART_Task",Task_Stack_Size, NULL,2, &UART_Task_handle);
    xTaskCreate(TaskState1, "TaskState1",Task_Stack_Size, NULL,3, &TaskState1_handle);


    /* Start Scheduler */
    vTaskStartScheduler();

}






