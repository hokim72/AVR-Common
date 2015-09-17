#include "uart.h"
#include "rprintf.h"
#include "debug.h"
#include "timer.h"
#include "mmc.h"

void mmc2Test(void);

int main(void)
{
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	// initialize the timer system
	timerInit();

	// print welcom message
	rprintfProgStrM("\r\nWelcome to the SD Test Suit!\r\n");

	mmc2Test();

	return 0;
}

void mmc2Test(void)
{
	uint32_t sector=0;
	uint8_t buffer[1024];
	uint16_t i;


	// initialize MMC interface
	mmcInit();
	mmcReset();
	for (i=0; i<1024; i++) buffer[i]=1;
	mmcWrite(buffer, sector, 2);
	for (i=0; i<1024; i++) buffer[i]=0;
	mmcRead(buffer, sector, 2);
	debugPrintHexTable(1024, buffer);
}
