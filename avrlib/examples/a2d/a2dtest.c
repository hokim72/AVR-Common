#include "uart.h"
#include "rprintf.h"
//#include "timer.h"
#include "a2d.h"
#include <util/delay.h>

int main(void)
{
	uint16_t a = 0;
	uint8_t i = 0;

	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	// initialize the timer system
	//timerInit();
	// turn on and initialize A/D converter
	a2dInit();

	// print a little intro message so we know things are working
	rprintf("Welcome to the a2d test!\r\n");

	// configure a2d port (PORTC) as input
	// so we can receive analog signals
	DDRC = 0x00;
	// make sure pull-up resistors are turned off
	PORTC = 0x00;

	// set the a2d prescaler (clock division ratio)
	// - a lower prescale setting will make the a2d converter go faster
	// - a higher setting will make it go slower but the measurements
	// 	 will be more accurate
	// - other allowed prescale values can be found in a2d.h
	a2dSetPrescaler(ADC_PRESCALE_DIV32);

	// set the a2d reference
	// - the reference is the voltage against which a2d measurements are made
	// - other allowed reference values can be found in a2d.h
	a2dSetReference(ADC_REFERENCE_AVCC);

	// use a2dConvert8bit(channel#) to get an 8bit a2d reading
	// use a2dConvert10bit(channel#) to get a 10bit a2d reading

	while(1)
	{
		// sample all a2d channels and print them to the terminal
		for (i=0; i<6; i++)
		{
			rprintf("Channel %d: %d  \r\n", i, a2dConvert8bit(i));
		}
		// print the sample number so far
		rprintf("Sample # : %d  \r\n", a++);
		_delay_ms(1000);
	}

	return 0;
}

