#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "extint.h"

// Global variables
typedef void (*voidFuncPtr)(void);
volatile static voidFuncPtr ExtIntFunc[EXTINT_NUM_INTERRUPTS];

// fucntions

// initializes extint library
void extintInit(void)
{
	uint8_t intNum;
	// detach all user functions from interrupts
	for (intNum=0; intNum<EXTINT_NUM_INTERRUPTS; intNum++)
		extintDetach(intNum);
}

// Configure external interrupt trigger
// NOTE: this function is not complete!!!
void extintConfigure(uint8_t interruptNum, uint8_t configuration)
{
	if (interruptNum == EXTINT0)
	{
		EICRA &= ~((1<<ISC01) | (1<<ISC00));
		EICRA |= configuration;
	}
	#ifdef INT1_vect
	else if (interruptNum == EXTINT1)
	{
		EICRA &= ~((1<<ISC11) | (1<<ISC10));
		EICRA |= configuration<<2;
	}
	#endif
	// need to handle a lot more cases
	// and differences between processors.
	// looking for clean way to do it...
}

// Attach a user function to an external interrupt
void extintAttach(uint8_t interruptNum, void (*userHandler)(void))
{
	// make sure the interrupt number is within bounds
	if (interruptNum < EXTINT_NUM_INTERRUPTS)
	{
		// set the interrupt function to run
		// the supplied user's function
		ExtIntFunc[interruptNum] = userHandler;
	}
}

// Detach a user function from an external interrupt
void extintDetach(uint8_t interruptNum)
{
	// make sure the interrupt number is within bounds
	if (interruptNum < EXTINT_NUM_INTERRUPTS)
	{
		// set the interrupt function to run
		// the supplied user's function
		ExtIntFunc[interruptNum] = 0;
	}
}

// Interrupt handler for INT0
ISR(INT0_vect)
{
	// if a user function is defined, execute it
	if (ExtIntFunc[EXTINT0])
		ExtIntFunc[EXTINT0]();
}

#ifdef INT1_vect
// Interrupt handler for INT1
ISR(INT1_vect)
{
	// if a user function is defined, execute it
	if (ExtIntFunc[EXTINT1])
		ExtIntFunc[EXTINT1]();
}
#endif
