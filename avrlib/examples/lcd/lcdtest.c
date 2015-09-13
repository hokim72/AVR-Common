#include "uart.h"
#include "rprintf.h"
#include "a2d.h"
#include "lcd.h"
#include <util/delay.h>

int main(void)
{
	uint8_t a=0;

	// intialize our libraries
	// intialize the UART (serial port)
	uartInit();
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	// turn on and initialize A/D converter
	a2dInit();
	// print a little intro message so we know things are working
	rprintf("\r\nWelcome to AVRlib!\r\n");

	// intiailize LCD
	lcdInit();
	// direct printf output to LCD
	rprintfInit(lcdDataWrite);

	// print message on LCD
	rprintf("Welcome to AVRlib!");

	DDRC = 0x00;
	PORTC = 0x00;

	// display a bargraph of the analog voltage on a2d channels 0, 1
	while (1)
	{
		lcdGotoXY(0,0);
		lcdProgressBar(a2dConvert8bit(0), 255, 10);
		rprintf(" X: %d", a2dConvert8bit(0));
		rprintf(" Sample: %d", a++);
		lcdGotoXY(0,1);
		lcdProgressBar(a2dConvert8bit(1), 255, 10);
		rprintf(" Y: %d", a2dConvert8bit(1));
		_delay_ms(500);		
	}

	return 0;
}
