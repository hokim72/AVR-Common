// This uart library emulates the operation of a UART (serial port) using
// the AVR's hardware timers, I/O pins, and some software.
//
// Specifically, this code uses:
// 		- Timer 1 Output Compare A for transmit timing
// 		- Timer 1 Output Compare B for receive timing
// 		- Timer 1 Input Capture for receive triggering
//
// The above resources cannot be used for other purposes while this software
// UART is enabled. The overflow interrupt from Timer1 can still be used for
// other timing, but the prescaler for Timer1 must not be changed.
//
// Serial output from this UART can be routed to any I/O pin. Serial input for
// this UART must come from the Timer1 Input Capture (IC1) I/O pin.
// These options should be configured by editing your local copy of
// "uartswconf.h".

#ifndef UARTSW_H
#define UARTSW_H

#include "global.h"
#include "buffer.h"
#include <avr/io.h>

// include configuration
#include "uartswconf.h"

// constants/macros/typdefs

// functions

// enable and initialize the software uart
void uartswInit(void);
// create and initialize the uart buffers
void uartswInitBuffers(void);
// turns off software UART
void uartswOff(void);
// returns the receive buffer structure
cBuffer* uartswGetRxBuffer(void);
// sets the uart baud rate
void uartswSetBaudRate(uint32_t baudrate);
// sends a single byte over the uart
void uartswSendByte(uint8_t data);

// Gets a single byte from the uart receive buffer
// Return the byte, or -1 if no byte is available (getchar-style)
int uartswGetByte(void);

// gets a single byte from the uart receive buffer
// Function returns TRUE if data was available, FALSE if not.
// Actual data is returned in variable pointed to by "data".
// example usage:
// char myReceivedByte;
// uartswReceiveByte( &myReceivedByte );
uint8_t uartswReceiveByte(uint8_t* rxData);

// internal transmit bit handler
void uartswTxBitService(void);
// internal receive bit handler
void uartswRxBitService(void);

#endif
