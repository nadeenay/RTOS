
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

#define USE_Timers

/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"



/* Global variable */
uint32_t technique;
uint8_t Dir=0;

/* Create Queue handle */
QueueHandle_t Q;

/* Create Timer handle */
TimerHandle_t TM;

/* Create event group handle */
EventGroupHandle_t E;

/*Define the stack size of the Tasks*/
#define Task_Stack_Size 128


/*-----------------------------------> Tasks <-----------------------------------*/

void UART0_Task(void * para)
{

    while(1)
    {

    /* check if there is a value to receive */
    if(UARTCharsAvail(UART0_BASE)) /*There is an Char in the FIFO*/
        {
           uint32_t Rec_Char =(uint32_t)(UARTCharGetNonBlocking(UART0_BASE));
           xQueueSend(Q,( void *)&Rec_Char,(TickType_t)0);
           UARTCharPut(UART0_BASE,Rec_Char);
           UARTCharPut(UART0_BASE,'\r');
           UARTCharPut(UART0_BASE,'\n');
        }
    vTaskDelay(100); /* Delay for 100ms */
    }
}


void RGB_Task(void * para)
{
    uint32_t Rec_Char;
    xQueueReceive(Q,(void*)&Rec_Char,(TickType_t)portMAX_DELAY);
    technique=Rec_Char;
}


void Button1_Task(void * para)
{
            int32_t data = 0;
            int32_t pre= 0x01 ; //to work on falling edge
            while(1)
            {
                data=GPIOPinRead(GPIO_PORTF_BASE,(uint8_t)GPIO_PIN_4);

                if((data & GPIO_PIN_4)==0 & (pre ==1)) //active low , if button is pressed , and last time was high
                {
                    xEventGroupSetBits( E,(EventBits_t)GPIO_PIN_4 );
                }
                pre = data;
                vTaskDelay(10); //ever 25 tick, as 25 ms
            }
}

void Button2_Task(void * para)
{
                int32_t data = 0;
                int32_t pre= 0x01 ; /* to work on falling edge */
                while(1)
                {
                    data=GPIOPinRead(GPIO_PORTF_BASE,(uint8_t)GPIO_PIN_0);

                    if((data & GPIO_PIN_0)==0 & (pre ==1)) /* active low , if button is pressed , and last time was high */
                    {
                        xEventGroupSetBits( E,(EventBits_t)GPIO_PIN_0 );
                    }
                    pre = data;
                    vTaskDelay(10); /* ever 25 tick, as 25 ms */
                }

}

void Action_Task(void * para)
{

 xEventGroupWaitBits(E,(EventBits_t)(GPIO_PIN_0|GPIO_PIN_4),pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
 static TickType_t period=100;
 switch(technique)
 {

 case 'r'|'b'|'g' :
 period/=2;    /* Increase the timer speed */
 if(period<1)
 period=100;
 xTimerChangePeriod( TM,period,(TickType_t)0 );
 break;


 case 'l':
     period=100;
     xTimerChangePeriod( TM,period,(TickType_t)0 );
     Dir^=1; /*toggle the direction of illuminating the LEDs*/
     break;

 default : break;

 }

}

/* Timer call back function */
void Timer_IS(TimerHandle_t TM)
{
    /*static variables */
    static uint32_t LED =GPIO_PIN_1;
    static uint32_t i=1;
    static uint32_t state1 =GPIO_PIN_1;
    static uint32_t state2 =GPIO_PIN_2;
    static uint32_t state3 =GPIO_PIN_3;

    switch(technique)
    {
    /*Red LED case */
    case 'a' :
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,state1);
    state1^=0xFF;
    break;

    /*Blue LED case */
    case 'b' :
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,state2);
    state2^=0xFF;
    break;

    /*Green LED case */
    case 'g' :
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,state3);
    state3^=0xFF;
    break;

    /* All LEDs illuminate case */
    case 'l' :
    GPIOPinWrite(GPIO_PORTF_BASE,LED,LED);
    LED+=i*2;
    i++;
    if(i>3)              /*Three LEDs have illuminated so start from red LED again*/
    {
        LED-=0x00000006; /* return to the GPIO_PIN_1 */
        i-=3;            /*return to i=1 */
    }
    break;

    default : break;
   }
}
void Task3(){

   /*UART configuration */
   UART0_Config();

   /*Enable clock for portF*/
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
   /* Unlock Pins of the SW */
   GPIOUnlockPin(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);
   /* Define the SW as Input pins */
   GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0 | GPIO_PIN_4);

   /*Define LEDs
    *  as OUTPUTs */
   GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1 |GPIO_PIN_2|GPIO_PIN_3);


    /* Create Handles */
    TaskHandle_t UART_handle,RGB_handle,Button1_handle,Button2_handle,Action_handle;

    /* Create 5 Tasks */
    xTaskCreate(UART0_Task, "UART_Task",Task_Stack_Size, NULL,4, &UART_handle);
    xTaskCreate(RGB_Task, "RGB_Task",Task_Stack_Size, NULL,3, &RGB_handle);
    xTaskCreate(Button1_Task, "Button1_Task",Task_Stack_Size, NULL,1, &Button1_handle);
    xTaskCreate(Button2_Task, "Button2_Task",Task_Stack_Size, NULL,2, &Button2_handle);
    xTaskCreate(Action_Task, "Action_Task",Task_Stack_Size, NULL,5, &Action_handle);


    /*Create queue */
    Q=xQueueCreate( 2,sizeof(uint8_t) );

    /*Create Timer*/
    TM=xTimerCreate("Timer",(TickType_t)100,pdTRUE, ( void *)0,Timer_IS );

    /*Start Timer */
    xTimerStart( TM,(TickType_t)0 );

    /*Create Event group*/
    E=xEventGroupCreate();

    /* Start Scheduler */
    vTaskStartScheduler();

}








