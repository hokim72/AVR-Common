#include "uart.h"
#include "rprintf.h"
#include "spieeprom.h"

uint8_t getNumber(void) {
	char hundreds = '0';
	char tens = '0';
	char ones = '0';
	int thisChar = '0';
	do {
		hundreds = tens;
		tens = ones;
		ones = (uint8_t)thisChar;
		do {
			thisChar=uartGetByte();
			if (thisChar == '\n') break;
		} while ((thisChar<'0') || (thisChar>'9'));
		uartSendByte((uint8_t)thisChar);
	} while (thisChar !=  '\n');
	return (100 * (hundreds - '0') + 10 * (tens - '0') + ones - '0');
}
int main(void)
{
	uint8_t i;
	int com;
	uint8_t address;

	uartInit();
	spieepromInit();
	rprintfInit(uartSendByte);

	while (1) 
	{
		rprintf("\r\n==== EEPROM Memory Playground ===\r\n");
		rprintf("Address Value\r\n");
		for (i=0; i<10; i++) {
			rprintf("  %03d    %03d\r\n", i, spieepromReadByte((uint16_t)i));
		}
		rprintf(" [e] to erase all memory\r\n");
		rprintf(" [w] to write byte to memory\r\n");

		while ((com=uartGetByte()) == -1);

		switch (com) {
			case 'e':
				rprintf("Clearing EEPROM, this could take a few seconds.\r\n");
				spieepromClearAll();
				break;
			case 'w':
				rprintf("Which memory slot would you like to write to?\r\n");
				address = getNumber();
				rprintf("\r\nWhat number would you like to store there?\r\n");
				i = getNumber();
				rprintf("address=%d, i=%d\r\n", address, i);
				spieepromWriteByte(address, i);
				break;
			default:
				rprintf("What??\r\n");
		}
	}

	return 0;
}
