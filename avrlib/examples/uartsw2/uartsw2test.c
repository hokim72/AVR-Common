#include "uartsw2.h"
//#include <util/delay.h>


int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartswInit();
	// set the baud rate of the UART for our debug/reporing output
	uartswSetBaudRate(9600);

	int serialChar;
	//serialChar='0';
	// should be transmit the first byte after power recycle.
	// Otherwise, you should lost the first received byte.
	// serialChar = '0';
	// loop_until_bit_is_set(UCSR0A, UDRE0);
	// UDR0 = (uint8_t)serialChar;
	while (1) {
		while ((serialChar = uartswGetByte()) == -1);
		uartswSendByte((uint8_t)serialChar);
		//_delay_ms(1000);
	}
	return 0;
}
