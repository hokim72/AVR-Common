#include "uart.h"
#include "rprintf.h"
#include "debug.h"
#include "sd.h"

void sdTest(void);

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

	sdTest();
	
	
	return 0;
}

void sdTest(void)
{
	uint32_t sector=0;
	uint8_t buffer[512];
	int c;

	// initialize SD interface
	sdInit();

	// print new prompt
	rprintf("\r\ncmd>");

	// testing loop
	while (1)
	{
		// check for keypress
		if((c=uartGetByte()) != -1)
		{
			switch(c)
			{
			case 'i':
				// initialze card
				rprintf("\r\nResetting SD Card\r\n");
				sdReset();
				break;
			case 'r':
				// read current sector into buffer
				rprintf("\r\nRead Sector %d\r\n", sector);
				sdReadSingleBlock(sector, buffer);
				// print buffer contents
				debugPrintHexTable(512, buffer);
				break;
			case 'w':
				// write current sector with data from buffer
				rprintf("\r\nWrite Sector %d\r\n", sector);
				sdWriteSingleBlock(sector, buffer);
				break;
			// move to new sector
			case '+': sector++;		rprintf("\r\nSector = %d", sector); break;
			case '-': sector--;		rprintf("\r\nSector = %d", sector); break;
			case '*': sector+=512;	rprintf("\r\nSector = %d", sector); break;
			case '/': sector-=512;	rprintf("\r\nSector = %d", sector); break;
			case '\n':
			default:
				break;
			}
			// print new prompt
			rprintf("\r\ncmd>");
		}
	}

}
