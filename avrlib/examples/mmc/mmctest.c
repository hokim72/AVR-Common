#include "uart.h"
#include "rprintf.h"
#include "debug.h"
#include "timer.h"
#include "mmc.h"

void mmcTest(void);

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

	mmcTest();

	return 0;
}

void mmcTest(void)
{
	uint32_t sector=0;
	uint8_t buffer[512];
	int c;

	// initialize MMC interface
	mmcInit();

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
                rprintf("\r\nResetting MMC Card\r\n");
                mmcReset();
                break;
            case 'r':
                // read current sector into buffer
                rprintf("\r\nRead Sector %d\r\n", sector);
                mmcRead(buffer, sector, 1);
                // print buffer contents
                debugPrintHexTable(512, buffer);
                break;
            case 'w':
                // write current sector with data from buffer
                rprintf("\r\nWrite Sector %d\r\n", sector);
                mmcWrite(buffer, sector, 1);
                break;
            // move to new sector
            case '+': sector++;     rprintf("\r\nSector = %d", sector); break;
            case '-': sector--;     rprintf("\r\nSector = %d", sector); break;
            case '*': sector+=512;  rprintf("\r\nSector = %d", sector); break;
            case '/': sector-=512;  rprintf("\r\nSector = %d", sector); break;
            case '\n':
            default:
                break;
            }
            // print new prompt
            rprintf("\r\ncmd>");
        }
    }
	
}
