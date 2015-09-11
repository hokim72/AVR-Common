#include "uart.h"


int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// set the baud rate of the UART for our debug/reporing output
	uartSetBaudRate(9600);

	int serialChar;
	// should be transmit the first byte after power recycle.
	// Otherwise, you should lost the first received byte.
	// serialChar = '0';
	// loop_until_bit_is_set(UCSR0A, UDRE0);
	// UDR0 = (uint8_t)serialChar;
	while (1) {
		while ((serialChar = uartGetByte()) == -1);
		uartSendByte((uint8_t)serialChar);
	}
	return 0;
}
