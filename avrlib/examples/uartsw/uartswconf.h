// This uart library emulates the operation of a UART (serial port) using
// the AVR's hardware timers, I/O pins, and some software.
//
// Specifically, this code uses:
// 		- Timer 1 Output Compare A for transmit timing
// 		- Timer	1 Output Compare B for receive timing
// 		- Timer	1 Input Capture for receive triggering
//
// The above resources cannot be used for other purposes while this software
// UART is enabled. The overflow interrupt from Timer1 can still be used for
// other timing, but the prescaler for Timer1 must not be changed.
//
// Serial output from this UART can be routed to any I/O pin. Serial input
// for this UART must come from the Timer1 input Capture (IC1) I/O pin.
// These options should be configured by editing your local copy of
// "uartswconf.h".

#ifndef UARTSWCONF_H
#define UARTSWCONF_H

// constants/macros/typdefs

#define UARTSW_RX_BUFFER_SIZE	0x20	// UART receive buffer size in bytes

//#define UARTSW_INVERT					// define to invert polarity of RX/TX signals
// When non-inverted, the serial line is appropriate for passing through
// an RS232 driver like the MAX232. When inverted, the serial line can
// directly drive/receive RS232 signals to/from a DB9 connector. Be sure
// to use a current-limiting resistor and perhaps a diode-clamp circuit when
// connecting incoming RS232 signals to a micorprocessor I/O pin

// if non-inverted, the serial line idles high (logic 1) between bytes
// if inverted, the serial line idles low (logic 0) between bytes

// UART transmit pin defines
#define	UARTSW_TX_PORT				PORTB		// UART Transmit Port
#define UARTSW_TX_DDR				DDRB		// UART	Transmit DDR
#define	UARTSW_TX_PIN				PB1			// UART	Transmit Pin

// UART receive pin define
// This pin must correspond to the
// Timer1 Input Capture (ICP or IC1) pin for your processor
#define UARTSW_RX_PORT				PORTB		// UART	Receive Port
#define	UARTSW_RX_DDR				DDRB		// UART	Receive DDR
#define UARTSW_RX_PORTIN			PINB		// UART Receive Port Input
#define UARTSW_RX_PIN				PB0			// UART Receive Pin

#endif

