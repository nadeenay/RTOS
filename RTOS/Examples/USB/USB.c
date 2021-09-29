
#ifndef EXAMPLES_USB_USB_C_
#define EXAMPLES_USB_USB_C_

#include "USB.h"


/* FreeRTOS*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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



/*Global flag indicating that a USB configuration has been set.*/
extern volatile bool g_bUSBConfigured;

extern QueueHandle_t Q;


/*
 * Function Name: USB_setLineCoding
 *
 * This function is called by the control handler when the get line event is passed to it
 * to set the serial communication port between the host (PC) and the device (Tiva board)
 */

static void USB_setLineCoding(tLineCoding *psLineCoding)
{

    psLineCoding->ui32Rate = 115200; /*Setting the baudrate*/

    psLineCoding->ui8Databits = 8; /*data length is 8*/

    psLineCoding->ui8Parity = USB_CDC_PARITY_NONE; /*No Parity*/

    psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;    /* 1 stop bit*/
}

/***************************************************************************************/
/*these handlers are High level handlers for the USB buffers
 * what does it mean ?
 * when the data sent from the USB host this data go to the bulk endpoint then
 * there is an Receive event will happen to notify the USB buffers that there is data
 * they should receive in the Rxbuffer this is the first event then the second event to
 * inform the  CDC device that there is a data in the Rxbuffer the application can receive it */

/*what happen if the data send from the USB host ? Not sure !!
 * 1-data go to the bulkOut Endpoint (lower layer of the USB) between USB driver and the USB host
 * 2-the function in g_sRxBuffer (buffer created for the application ) which is called USBDCDCPacketRead will be called to receive the data from USB endpoint
 * 3-this function puts the data in g_pui8USBRxBuffer in g_sRxBuffer
 * 4- USBBufferEventCallback handler then will be called
 * 5- this handler put the data in the ringbuffers witch the application can get data from it
 * 6- RxHandler will be called (in receive handler put the code you want for specific APP)
 */

/***************************************************************************************/




/*****************************************************************************
 Function Name: ControlHandler
 Handles CDC driver notifications related to control and setup of the device.
 \param pvCBData is the client-supplied callback pointer for this channel.
 \param ui32Event identifies the event we are being notified about.
 \param ui32MsgValue is an event-specific value.
 \param pvMsgData is an event-specific pointer.
 This function is called by the CDC driver to perform control-related
 operations on behalf of the USB host.  These functions include setting
 and querying the serial communication parameters, setting handshake line
 states and sending break conditions.
 \return The return value is event-specific.
*****************************************************************************/
uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
    /* Which event are we being asked to process?*/
    switch(ui32Event)
    {
        /* We are connected to a host and communication is now possible.*/
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;
            USBBufferFlush(&g_sTxBuffer);   /* Flush our buffers.*/
            USBBufferFlush(&g_sRxBuffer);
            break;

        case USB_EVENT_DISCONNECTED: /*The host has disconnected.*/
            g_bUSBConfigured = false;
            break;

        case USBD_CDC_EVENT_GET_LINE_CODING: /* Return the current serial communication parameters.*/
            USB_setLineCoding(pvMsgData);
            break;

        default:
            break;
    }
    return 0;
}

/*****************************************************************************
 Function Name: TxHandler
 Handles CDC driver notifications related to the transmit channel (data to
 the USB host).
 \param ui32CBData is the client-supplied callback pointer for this channel.
 \param ui32Event identifies the event we are being notified about.
 \param ui32MsgValue is an event-specific value.
 \param pvMsgData is an event-specific pointer.
 This function is called by the CDC driver to notify us of any events
 related to operation of the transmit data channel (the IN channel carrying
 data to the USB host).
 We don't need this function in this specific application.
*****************************************************************************/
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,void *pvMsgData)
{

    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:

            /* Since we are using the USBBuffer, we don't need to do anything
             here. */

            break;


        default:
            break;


    }
    return 0;
}

/*****************************************************************************
 Function Name: RxHandler
 Handles CDC driver notifications related to the receive channel (data from
 the USB host).
 \param ui32CBData is the client-supplied callback data value for this channel.
 \param ui32Event identifies the event we are being notified about.
 \param ui32MsgValue is an event-specific value.
 \param pvMsgData is an event-specific pointer.
 This function is called by the CDC driver to notify us of any events
 related to operation of the receive data channel (the OUT channel carrying
 data from the USB host).
 !NOTE: In this handler we only need the receive event. you can add more events in this
          function.
 \return The return value is event-specific.
*****************************************************************************/

uint8_t Rec_char;
uint8_t New_Line[]={'0','\r','\n'};

uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,void *pvMsgData)
{


    if(ui32Event == USB_EVENT_RX_AVAILABLE) /*Incoming data event*/
    {

        USBBufferRead((tUSBBuffer *)&g_sRxBuffer, &Rec_char, 1);
        New_Line[0]=Rec_char;
        USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,&New_Line, 3);

        /* Send the char to the Queue to be available for RGB task*/
        xQueueSendFromISR( Q,(const void *)&Rec_char,NULL);
    }

    return 0;
}


/*public functions */
void USB_serialInit()
{

    /*Configure the required pins for USB operation.*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

    /*Initialize the transmit and receive buffers.*/
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    /*Set the USB stack mode to Device mode with VBUS monitoring.*/
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    /*Pass our device information to the USB library and place the device
    on the bus.*/
    USBDCDCInit(0, &g_sCDCDevice);

}



#endif /* EXAMPLES_USB_USB_C_ */
