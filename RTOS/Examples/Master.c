
/*Description task :
 * In this task we have 4 Tasks run by the Scheduler with different priority and a Timer with priority (higher than the Tasks).
 * the User have to Enter :
 * r-->for blink the red LED
 * b-->for blink the blue LED
 * g-->for blink the green LED
 * l-->illuminate all reds in arrangement red-->blue-->green
 * R-->illuminate all reds in arrangement green-->blue-->red
 * -----------------------------------------------------------------<>--------------------------------------------------------------
 * the user have an option to Make action for the blinking led if he/she press switch2 :
 * if we in r/g/b case the led will blink faster
 * if we in l/R  case the arrangement of illuminating the LEDs will change to the other case ( l to R and vice versa). :)
 *
 * */
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

/*Create global variable */
uint8_t technique='r'; /* Initialize the variable with r */
uint32_t LED[] ={GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3};

TimerHandle_t TM;

EventGroupHandle_t Group1;



void MyTimer( TimerHandle_t Timer_Function1 )
{
    /*
    static uint32_t state =GPIO_PIN_1;
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,state);
    state ^= GPIO_PIN_1;
    */
    /*static variables */
        static uint32_t l=0;
        static int32_t R=2;
        static uint32_t state1 =GPIO_PIN_1;
        static uint32_t state2 =GPIO_PIN_2;
        static uint32_t state3 =GPIO_PIN_3;

        switch(technique)
        {
        /*Red LED case */
        case 'r' :
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,state1);
        state1^=GPIO_PIN_1;
        break;

        /*Blue LED case */
        case 'b' :
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,state2);
        state2^=GPIO_PIN_2;
        break;

        /*Green LED case */
        case 'g' :
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,state3);
        state3^=GPIO_PIN_3;
        break;

        /* All LEDs illuminate case */
        case 'l' :
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,LED[l]);
        l++;
        if(l>2)
            l-=3;
        break;

        case 'R':
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,LED[R]);
             R--;
           if(R<0)
             R+=3;
        break;
        default : break;
       }
}
void Uart_Task(void* para)
{


    UART0_Send_String("please enter character r or g or b");
    while(1)
    {

        /* check if there is a value to receive */
        if(UARTCharsAvail(UART0_BASE)) /*There is an Char in the FIFO*/
            {

               technique =(uint8_t)(UARTCharGetNonBlocking(UART0_BASE));
               UARTCharPut(UART0_BASE,technique);
               UARTCharPut(UART0_BASE,'\r');
               UARTCharPut(UART0_BASE,'\n');
         }

    }
}

void Action_Task(void * para)
{
    TickType_t period=100;
    while(1)
    {

    xEventGroupWaitBits(Group1,GPIO_PIN_0,pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
    switch(technique)
   {

   case 'r':
   case 'b':
   case 'g':
   period/=2;    /* Increase the timer speed */
   if(period<1)
   period=300;
   xTimerChangePeriod( TM,period,(TickType_t)100 );
   break;


   case 'l': /* the LEDs illuminate as red--->blue --->green */
   case 'R': /* the LEDs illuminate as green--->blue --->red */
   period=300;
   xTimerChangePeriod( TM,period,(TickType_t)100 );
   xEventGroupSetBits(Group1,0x20);
   break;

 default : break;

 }
 }

}


void Change_Dir_Task(void * para)
{

    while(1)
    {
    /* wait for pin 5 to be set */
    xEventGroupWaitBits(Group1,GPIO_PIN_5,pdTRUE,pdTRUE,(TickType_t)portMAX_DELAY );
    if(technique=='l')
        technique='R';
    else  if(technique=='R')
        technique='l';

    }

}


void Switch2_Task(void * para)
{
    uint32_t data = 0;
    uint8_t pre= 1 ;
    while(1)
    {
        data=GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);

        if((data & GPIO_PIN_0)==0 & (pre ==1)) /* Detect the falling edge */
        {
            xEventGroupSetBits(Group1,0x01);
        }
        pre = (data & GPIO_PIN_0);
        vTaskDelay(10);
    }

}

void Master() {
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

    /* Handle Tasks */
    TaskHandle_t  Uart_handle,Switch2_handle,Action_Task_handle,Change_Dir_handle;

    /* Create timer */
    TM= xTimerCreate(  "MyTimer",300, pdTRUE,( void * ) 0, MyTimer);

    /* Create Event group */
    Group1 = xEventGroupCreate();

    /* Start Timer */
    xTimerStart( TM, 0 );

    /* Create Tasks */
    xTaskCreate(Uart_Task, "Uart_Task",SWITCHTASKSTACKSIZE, NULL,1, &Uart_handle);
    xTaskCreate(Switch2_Task, "Switch2",SWITCHTASKSTACKSIZE, NULL,2, &Switch2_handle);
    xTaskCreate( Action_Task, " Action_Task",SWITCHTASKSTACKSIZE, NULL,3, &Action_Task_handle);
    xTaskCreate( Change_Dir_Task, " Change_Dir_Task",SWITCHTASKSTACKSIZE, NULL,4, &Change_Dir_handle);

    /* Start Scheduler */
    vTaskStartScheduler();

}
