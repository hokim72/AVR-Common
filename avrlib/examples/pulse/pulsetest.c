#include "uart.h"
#include "rprintf.h"
#include "timer.h"
#include "pulse.h"

void pulseTest(void);

int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// set the baud rate of the UART for our debug/reporting output
	uartSetBaudRate(9600);
	// initialize the timer system
	timerInit();
	// initialize rprintf system
	rprintfInit(uartSendByte);

	// run the test
	pulseTest();

	return 0;
}

void pulseTest(void)
{
	uint16_t i;

	// print a little intro message so we know things are working
	rprintf("\r\nWelcome to the pulse library test program!\r\n");

	// set pulse pins as output
	// ** these are the correct pins for most processors, but not all **
	sbi(DDRB, 1);
	sbi(DDRB, 2);

	// initialize pulse library
	pulseInit();

	// do logarithmic frequency sweep
	rprintf("Running logarithmic frequency sweep...\r\n");
	for (i=1; i<4000; i+=((i/5)+1))
	{
		// set the output frequency
		pulseT1ASetFreq(i);
		// - set the number of pulse to output
		// - number of pulse will be set to make each
		//   frequency last 1/5 of a second in time
		// ** make sure we don't request zero pulses
		// ** because this means run indefinitely
		pulseT1ARun( (i/5)+1 );
		// print a debug message
		rprintf("Output Freq = %dHz\r\n", i);
		// wait for pulses to finish being output
		while(pulseT1ARemaining());
	}

	// do simultaneous pulse output on OC1A and OC1B pins with different frequencies
	rprintfCRLF();
	rprintf("Running simultaneous output on OC1A and OC1B with different frequencies\r\n");
	rprintf("OC1A will be at 50Hz for 2 seconds (100 pulses)\r\n");
	rprintf("OC1B will be at 175Hz for 4 seconds (700 pulses)\r\n");
	// start OC1A output
	pulseT1ASetFreq(50);
	pulseT1ARun(100);
	// start OC1B output
	pulseT1BSetFreq(175);
	pulseT1BRun(700);
	// enter a debug loop as long as there are pulses remaining
	while (pulseT1ARemaining() | pulseT1BRemaining())
	{
		rprintf("OC1A has %d pulses remaining, OC1B has %d pulses remaining\r\n", pulseT1ARemaining(), pulseT1BRemaining());
		timerPause(100);
	}
	rprintf("done!\r\n");
}
