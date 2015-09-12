#include "uart.h"
#include "rprintf.h"
#include "timer.h"
#include "extint.h"

// global variables
volatile uint16_t Int0Count;
volatile uint16_t Int1Count;

// functions
void extintTest(void);
void myInt0Handler(void);
void myInt1Handler(void);

int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// set the baud rate of the UART for our debug/reporting output
	uartSetBaudRate(9600);
	// initialize rprintf system
	rprintfInit(uartSendByte);
	// initialize timers
	timerInit();

	// run the test
	extintTest();

	return 0;
}

void extintTest(void)
{
	uint16_t temp0, temp1;

	// print a little intro message so we know things are working
	rprintf("\r\n\n\nWelcome to the External Interrupt library test program!\r\n");

	// initialize the external interrupt library
	rprintf("Initialize external interrupt library\r\n");
	extintInit();

	// configure external interrupts for rising-edge triggering.
	// when a rising-edge pulse arrives on INT0 or INT1,
	// the interrupt will be triggered
	rprintf("Configuring external interrupts\r\n");
	extintConfigure(EXTINT0, EXTINT_EDGE_RISING);
	extintConfigure(EXTINT1, EXTINT_EDGE_RISING);

	// attach user interrupt routines.
	// when the interrupt is triggered, the user routines will be executed
	rprintf("Attaching user interrupt routines\r\n");
	extintAttach(EXTINT0, myInt0Handler);
	extintAttach(EXTINT1, myInt1Handler);

	// enable the interrupts
	rprintf("Enabling external interrupts\r\n");
	// (support for this has not yet been added to the library)
	sbi(EIMSK, INT0);
	sbi(EIMSK, INT1);

	// In this loop we will count the number of external interrupts,
	// and therefore the number of rising edges, that occur in one second.
	// This is precisely the frequency, in cycles/second or Hz, of the signal
	// that is triggering the interrupt.

	while (1)
	{
		// reset interrupt counters
		Int0Count = 0;
		Int1Count = 0;
		// wait 1 second
		timerPause(1000);
		// get counter values
		temp0 = Int0Count;
		temp1 = Int1Count;
		// print results
		rprintf("Frequency on INT0 pin: %dHz -- INT1 pint: %dHz\r\n", temp0, temp1);
	}
}

void myInt0Handler(void)
{
	// count this interrupt event
	Int0Count++;
}
void myInt1Handler(void)
{
	// count this interrupt event
	Int1Count++;
}
