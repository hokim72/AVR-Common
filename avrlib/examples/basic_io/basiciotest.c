#include <avr/io.h>
#include "global.h"

int main(void) {

	uint8_t a;

	// All AVR processors have I/O ports which each contain up to 8
	// user-controllable pins. From a hardware perspective, these I/O pins
	// are each an actual physical pin coming out of the processor chip.
	// The voltage on the pins can be sensed or controlled via software,
	// hence their designation as Input/Output pins.

	// While I/O pins are actual wires in the real world, they also exist
	// inside the processor as special memory locations called registers.
	// The software-controlled contents of these registers is what
	// determines the state and operation of I/O pins and I/O ports.

	// Since AVR processors deal naturally with 8 bits at a time, I/O pins
	// are grouped into sets of 8 to form I/O ports. Three registers
	// are assigned to each I/O port to control the function and state of
	// that port's pins. The registers are 8-bits wide, and each bit (#0-7)
	// determines the operation of the corresponding number pin (pin 0-7).
	// The three registers are:

	// DDRx	- this register determines the direction (input/output) of the pins on port[x]
	// 		  A '0' bit in the DDR makes that port pin act as input
	// 		  A '1' bit in the DDR makes that port pin act as output

	// PORTx - this register contains the output state of the pins on port[x]
	// 		   A '0' bit makes the port pin output a LOW (~0V)
	// 		   A '1' bit makes the port pin output a HIGH (~5V)

	// PINx	- this register contains the input state of the pins on port[x]
	// 		  A '0' bit indicates that the port pin is LOW (at ~0V)
	// 		  A '1' bit indicates that the port pin is HIGH (at ~5V)

	// The x should be replaced with A,B,C,D,E,F, or G depending on the
	// desired port. Note that not all AVR processors have the same set
	// or number of ports. Consult the datasheet for your specific processor
	// to find out which ports it has.

	// in the AVR-GCC C language, ports can be accessed using two kinds of
	// commands:
	// inb() and outb()   -   in-byte and out-byte
	// cbi() and sbi()	  -   clear-bit and set-bit
	
	// inb() and outb() should be used when you intend to read or write
	// serveral bits of a register at once. Here are some examples:

	outb(DDRB, 0x00);	// set all port B pins to input
	a = inb(PINB);		// read the input state of all pins on port B

	outb(DDRD, 0xFF);	// set all port D pins to output
	outb(PORTD, 0xF0);	// set PD4-7 to HIG and PD0-3 to LOW

	// Often you may wish to change only a single bit in the registers
	// while leaving the rest unaltered. For this, use cbi() and sbi().
	// For example:

	sbi(DDRB, 0);		// sets PB0 to be an output
	sbi(DDRB, 1);		// sets PB1 to be an output

	cbi(PORTB, 1);		// sets PB1 to output a LOW without altering any other pin

	// the lines below will cause PB0 to pulse twice,
	// but will leave all other port B pins unchanged
	cbi(PORTB, 0);		// sets PB0 to output a LOW
	sbi(PORTB, 0);		// sets PB0 to output a HIGH
	cbi(PORTB, 0);		// sets PB0 to output a LOW
	sbi(PORTB, 0);		// sets PB0 to output a HIGH
	cbi(PORTB, 0);		// sets PB0 to output a LOW

	return 0;
}
