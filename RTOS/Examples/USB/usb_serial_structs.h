/*
 * usb_serial_structs.h
 *
 *  Created on: Sep 29, 2021
 *      Author: Nadeen
 */

#ifndef EXAMPLES_USB_USB_SERIAL_STRUCTS_H_
#define EXAMPLES_USB_USB_SERIAL_STRUCTS_H_

//*****************************************************************************
//
// The size of the transmit and receive buffers used for the redirected UART.
// This number should be a power of 2 for best performance.  256 is chosen
// pretty much at random though the buffer should be at least twice the size of
// a maxmum-sized USB packet.

/* the USB Endpoints can send or receive data at maximum packet size of 64 bytes (In the lower level )
 *  and as we need our Buffers (rxbuffer ,txbuffer) to be at size witch is at least twice the size of
 *a maxmum-sized USB packet and be power of two so it is fine to make it (64*2 =128) the power after 128
 *so it's 256
 *  */
//
//*****************************************************************************
#define USB_BUFFER_SIZE 256

extern uint32_t RxHandler(void *pvCBData, uint32_t ui32Event,
                          uint32_t ui32MsgValue, void *pvMsgData);
extern uint32_t TxHandler(void *pvi32CBData, uint32_t ui32Event,
                          uint32_t ui32MsgValue, void *pvMsgData);

extern tUSBBuffer g_sTxBuffer;
extern tUSBBuffer g_sRxBuffer;
extern tUSBDCDCDevice g_sCDCDevice;
extern uint8_t g_pui8USBTxBuffer[];
extern uint8_t g_pui8USBRxBuffer[];







#endif /* EXAMPLES_USB_USB_SERIAL_STRUCTS_H_ */
