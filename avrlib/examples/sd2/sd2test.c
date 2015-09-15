#include "uart.h"
#include "rprintf.h"
#include "debug.h"
#include "sd.h"

void sd2Test(void);

int main(void)
{
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);

	// print welcom message
	rprintfProgStrM("\r\nWelcome to the SD Test Suit!\r\n");

	sd2Test();
	
	
	return 0;
}

void sd2Test(void)
{
	uint8_t buffer[1024];

	// initialize SD interface
	sdInit();
	sdReset();

	#if 0
	sdWriteSingleBlock(0, buffer);
	sdWriteSingleBlock(1, buffer);
	sdWriteSingleBlock(2, buffer);
	sdWriteSingleBlock(3, buffer);
	sdReadSingleBlock(0, buffer);
	sdReadSingleBlock(1, buffer);
	sdReadSingleBlock(2, buffer);
	sdReadSingleBlock(3, buffer);
	sdReadMultipleBlock(0, 2, buffer);
	sdReadMultipleBlock(2, 2, buffer);
	#endif
	sdWriteMultipleBlock(0, 2, buffer);
}
