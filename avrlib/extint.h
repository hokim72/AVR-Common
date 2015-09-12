// This library provides convenient standardized  configuration and access to
// external interrupts. The library is designed to make it possible to write
// code that uses external interrupts without digging into the processor
// datasheets to find register names and bit-defines. The library also strives
// to allow code which uses external interrupts to more easily cross-compile
// between different microcontrollers.
//
// NOTE: Using this library has certain advantages, but also adds overhead and
// latency to interrupt servicing. If the smallest code size or fastest
// possible latency is needed, do NOT use this library; link your interrupts
// directly.

#ifndef EXTINT_H
#define EXTINT_H

#include "global.h"
#include <avr/io.h>

// constants/macros/typedefs

// interrupt macros for attaching user functions to external interrupts
// use these with extintAttach( intNum, function )
#define EXTINT0					0x00		// External Interrupt 0
#define EXTINT1					0x01		// External Interrupt 1

#define EXTINT_LEVEL_LOW		0x00		// Trigger on low level
#define EXTINT_EDGE_ANY			0x01		// Trigger on any edge
#define EXTINT_EDGE_FALLING		0x02		// Trigger on falling edge
#define EXTINT_EDGE_RISING		0x03		// Trigger on rising edge

#define EXTINT_NUM_INTERRUPTS	2

// functions

// initializes extint library
void extintInit(void);

// Configure external interrupt trigger
void extintConfigure(uint8_t interruptNum, uint8_t configuration);

// extintAttach and extintDetach commands
// 		These functions allow the attachment (or detachment) of any user
// 		function to an external interrupt. "Attaching" one of your own
// 		functions to an interrupt means that it will be called whenever
// 		that interrupt is triggered. Example usage:
//
// 		extintAttach(EXTINT0, myInterruptHandler);
// 		extintDetach(EXTINT0);
//
// 		extintAttach causes the myInterruptHandler() to be attached, and therefore
// 		execute, whenever the corresponding interrupt occurs. extintDetach removes
// 		the association and executes no user function when the interrupt occurs.
// 		myInterruptFunction must be defined with no return value and no arguments:
//
// 		void myInterruptHanlder(void) { ... }

// Attach a user function to an external interrupt
void extintAttach(uint8_t interruptNum, void (*userHandler)(void) );
// Detach a user function from an external interrupt
void extintDetach(uint8_t interruptNum);

#endif
