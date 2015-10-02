#include <avr/io.h>
#include <avr/interrupt.h>

#include "buffer.h"
#include "uart.h"

// UART global variables
// flag variables
//volatile uint8_t uartReadyTx;		// uartReadyTx flag
//volatile uint8_t uartBufferedTx;	// uartBufferedTx flag
// receive and transmit buffers
cBuffer uartRxBuffer;				// uart receive buffer
//cBuffer uartTxBuffer;				// uart transmit buffer
unsigned short uartRxOverflow;		// receive overflow counter

// using internal ram,
// automatically allocate space in ram for each buffer
static unsigned char uartRxData[UART_RX_BUFFER_SIZE];
//static unsigned char uartTxData[UART_TX_BUFFER_SIZE];

typedef void (*voidFuncPtru08)(unsigned char);
volatile static voidFuncPtru08 UartRxFunc;

// enable and initialize the uart
void uartInit(void)
{
	// initialize the buffers
	uartInitBuffers();
	// initialize user receive handler
	UartRxFunc = 0;

#if USE_2X
	sbi(UCSRA, U2X);
#else
	cbi(UCSRA, U2X);
#endif

	// enable RxD/TxD and interrupts
	//outb(UCSRB, BV(RXCIE)|BV(TXCIE)|BV(RXEN)|BV(TXEN));
	outb(UCSRB, BV(RXCIE)|BV(RXEN)|BV(TXEN));
	outb(UCSRC, BV(UCSZ1)|BV(UCSZ0)); /* 8 data bits, 1 stop bit */

	// set default baud rate
	uartSetBaudRate(UART_DEFAULT_BAUD_RATE);
	// initialize states
	//uartReadyTx = TRUE;
	//uartBufferedTx = FALSE;
	// clear overflow count
	uartRxOverflow = 0;
	// enable interrupts
	sei();
}

// create and initialize the uart transmit and receive buffers
void uartInitBuffers(void)
{
	// initialize the UART receive buffer
	bufferInit(&uartRxBuffer, uartRxData, UART_RX_BUFFER_SIZE);
	// initialize the UART transmit buffer
	//bufferInit(&uartTxBuffer, uartTxData, UART_TX_BUFFER_SIZE);
}

// redirects received data to a user function
void uartSetRxHandler(void (*rx_func)(unsigned char c))
{
	// set the receive interrupt to run the supplied user function
	UartRxFunc = rx_func;
}

// set the uart baud rate
void uartSetBaudRate(uint32_t baudrate)
{
	// calculate division factor for requested baud rate, and set it
	uint16_t bauddiv = ((F_CPU+(baudrate*8L))/(baudrate*16L)-1);
	outb(UBRRL, bauddiv);
	outb(UBRRH, bauddiv>>8);
}

// returns the receive buffer structure
cBuffer* uartGetRxBuffer(void)
{
	// return rx buffer pointer
	return &uartRxBuffer;
}

#if 0
// returns the transmit buffer structure
cBuffer* uartGetTxBuffer(void)
{
	// return tx buffer pointer
	return &uartTxBuffer;
}
#endif

// transmits a byte over the uart
void uartSendByte(uint8_t txData)
{
	// wait for the transmitter to be ready
	//while (!uartReadyTx);
	// send byte
	loop_until_bit_is_set(UCSRA, UDRE);
	outb(UDR, txData);
	// set ready state to FALSE
	//uartReadyTx = FALSE;
}

// gets a single byte from the uart receive buffer (getchar-style)
int uartGetByte(void)
{
	uint8_t c;
	if (uartReceiveByte(&c))
		return c;
	else
		return -1;
}

// gets a byte (if available) from the uart receive buffer
uint8_t uartReceiveByte(uint8_t* rxData)
{
	// make sure we have a receive buffer
	if (uartRxBuffer.size)
	{
		// make sure we have data
		if (uartRxBuffer.datalength)
		{
			// get byte from beginning of buffer
			*rxData = bufferGetFromFront(&uartRxBuffer);
			return TRUE;
		}
		else
		{
			// no data
			return FALSE;
		}
	}
	else
	{
		// no buffer
		return FALSE;
	}
}

// flush all data out of the receive buffer
void uartFlushReceiveBuffer(void)
{
	// flush all data from receive buffer
	// bufferFlush(&uartRxBuffer);
	// same effect as above
	uartRxBuffer.datalength = 0;
}

// return true if uart receive buffer is empty
uint8_t uartReceiveBufferIsEmpty(void)
{
	if (uartRxBuffer.datalength == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#if 0
// add byte to end of uart Tx buffer
uint8_t uartAddToTxBuffer(uint8_t data)
{
	// add data byte to the end of the tx buffer
	return bufferAddToEnd(&uartTxBuffer, data);
}

// start transmission of the current uart Tx buffer contents
void uartSendTxBuffer(void)
{
	// turn on buffered transmit
	uartBufferedTx = TRUE;
	// send the first byte to get things going by interrupts
	uartSendByte(bufferGetFromFront(&uartTxBuffer));
}

void uartSendTxBuffer(void)
{
	while (uartTxBuffer.datalength)
	{
		loop_until_bit_is_set(UCSRA, UDRE);
		UDR = bufferGetFromFront(&uartTxBuffer);
	}
}

// UART Transmit Complete Interrupt Handler
ISR(USART_TX_vect)
{
	// check if buffered tx is enabled
	if (uartBufferedTx)
	{
		// check if there's data left in the buffer
		if (uartTxBuffer.datalength)
		{
			// send byte from top of buffer
			outb(UDR, bufferGetFromFront(&uartTxBuffer));
		}
		else
		{
			// no data left
			uartBufferedTx = FALSE;
			// return to ready state
			uartReadyTx = TRUE;
		}
	}
	else
	{
		// we're using single-byte tx mode
		// indicate transmit complete, back to ready
		uartReadyTx = TRUE;
	}
}
#endif

// UART Receive Complete Interrupt Handler
ISR(USART_RX_vect)
{
	uint8_t c;

	// get received char
	c = inb(UDR);

	// if there's a user function to handle this receive event
	if (UartRxFunc)
	{
		// call it and pass the received data
		UartRxFunc(c);
	}
	else
	{
		// otherwise do default processing
		// put received char in buffer
		// check if there's space
		if (!bufferAddToEnd(&uartRxBuffer, c))
		{
			// no space in buffer
			// count overflow
			uartRxOverflow++;
		}
	}
}
