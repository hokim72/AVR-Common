// This uart library emulates the operation of a UART (serial port) using
// the AVR's hardware timers, I/O pins, and some software.
//
// Specifically, this code uses:
// 		- Timer 0 Output Compare A for transmit timing
// 		- Timer 0 Output Compare A for receive timing
// 		- External Interrupt 1 for receive triggering
//
// The above resources cannot be used for other purpose while this software
// UART is enabled. The overflow interrupt from Timer0 can still be used
// for other timing, but the prescaler for this timer must not be changed.
//
// Serial output from this UART can be routed to any I/O pin. Serial input
// for this UART must come from the External Interrupt 2 (INT2) I/O pin.
// These optins should be configured by editing your local copy of
// "uartsw2conf.h".

#ifndef UARTSW2_H
#define UARTSW2_H

#include "global.h"
#include "buffer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// include configuration
#include "uartsw2conf.h"

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
