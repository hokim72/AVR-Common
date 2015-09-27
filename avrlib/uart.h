// This libary provides both buffered and unbuffered transmit and receive
// functions for the AVR processor UART. Buffered access means that the
// UART can transmit and receive data in the "background", while your code
// continues executing. Also included are functions to initialize the
// UART, set the baud rate, flush the buffers, and check buffer status.
//
// For full text output functionality, you may wish to use the rprintf
// functions along with this driver.
//
// Most Atmel AVR-series processor contain one or more hardware UARTs
// (aka, serial ports). UART serial ports can communicate with other
// serial ports of the same type, like those used on PCs. In general,
// UARTs are used to communicate with devices that are RS-232 compatible
// (RS-232 is a certain kind of serial port).
//
// By far, the most common use for serial communications on AVR processors
// is for sending information and data to a PC running a terminal program.
// Here is an example:
//
// uartInit();					// initialize UART (serial port)
// uartSetBaudRate(9600);		// set UART speed to 9600 baud
// rprintfInit(uartSendByte);	// configure rprintf to use UART for output
// rprintf("Hello World\r\n");	// send "hello world" message via serial port
//
// The CPU frequency (F_CPU) must be set correctly for the UART library to
// calculate correct baud rates. Furthermore, certain CPU frequencies will
// not produce exact baud rates due to integer frequency division round-off.
// See your AVR processor's datasheet for full details.

#ifndef UART_H
#define UART_H

#include <inttypes.h>
#include "global.h"
#include "buffer.h"
//#include "uartconf.h"

// Default uart baud rate.
// This is the default speed after a uartInit() command,
// and can be changed by using uartSetBaudRate().
#define	UART_DEFAULT_BAUD_RATE	9600

// buffer memory allocation defines
// buffer sizes
#ifndef UART_TX_BUFFER_SIZE
// Number of bytes for uart transmit buffer.
// Do not change this value in uart.h, but rather override
// it with the desired value defined in your project's CMakeLists.txt
#define UART_TX_BUFFER_SIZE	0x0040
#endif
#ifndef UART_RX_BUFFER_SIZE
// Number of bytes for uart receive buffer.
// Do not change this value in uart.h, but rather override
// it with the desired value defined in your project's CMakeLists.txt
#define UART_RX_BUFFER_SIZE	0x0040
#endif

// functions

// Initializes uart.
// After running this init function, the processor
// I/O pins that used for uart communications (RXD, TXD)
// are no long available for general purpose I/O.
void uartInit(void);

// Initialize transmit and receive buffers.
// Automatically called from uartInit()
void uartInitBuffers(void);

// Redirects received data to a user function.
void uartSetRxHandler(void (*rx_func)(uint8_t c));

// Sets the uart baud rate.
// Argument should be in bits-per-second, like uartSetBaudRate(9600);
void uartSetBaudRate(uint32_t baudrate);

// Returns pointer to the receive buffer structure.
cBuffer* uartGetRxBuffer(void);

// Returns pointer to the transmit buffer structure.
cBuffer* uartGetTxBuffer(void);

// Sends a single byte over the uart.
// This functions waits for the uart to be ready,
// therefore, consecutive calls to uartSendByte() will
// go only as fast as the data can be sent over the
// serial port.
void uartSendByte(uint8_t data);

// Gets a signle byte from the uart receive buffer.
// Returns the byte, or -1 if no byte is available (getchar-style).
int uartGetByte(void);

// Gets a single byte from the uart receive buffer.
// Function return TRUE if data was available , FALSE if not.
// Actual data is returned in variable pointed to by "data".
// Example usage:
//
// char myReceivedByte;
// uartReceiveByte( &myReceivedByte );
uint8_t uartReceiveByte(uint8_t* data);

// Returns TRUE/FALSE if receive buffer is empty/not-empty.
uint8_t uartReceiveBufferIsEmpty(void);

// Flushes (deletes) all data from receive buffer.
void uartFlushReceiveBuffer(void);

// Add byte to end of uart Tx buffer.
// Returns TRUE if successful, FALSE if failed (no room left in buffer).
uint8_t uartAddToTxBuffer(uint8_t data);

// Begins transmission of the transmit buffer under interrupt control.
void uartSendTxBuffer(void);

// Sends a block of data via the uart using interrupt control.
// buffer : pointer to data to be sent
// nBytes : length of data (number of bytes to sent)
uint8_t uartSendBuffer(char* buffer, uint16_t nBytes);

#endif
