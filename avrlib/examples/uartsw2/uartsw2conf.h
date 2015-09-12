// This uart library emulates the operation of a UART (serial port) using
// the AVR's hardware timers, I/O pins, and some software.
//
// Specially, this code uses:
// 		-Timer 0 Output Compare A for transmit timing
// 		-Timer 0 Output Compare B for receive timing
// 		-External Interrupt 1 for receive triggering
//
// The above resources cannot be used for other purposes while this software
// UART is enabled. The overflow interrupt from Timer0 can still be used
// for other timing, but the prescaler for this timer must not be changed.
//
// Serial output from this UART can be routed to any I/O pin. Serial input
// for this UART must come from the External Interrupt 1 (INT1) I/O pin.
// These options should be configured by editing your local copy of
// "uartsw2conf.h"

#ifndef UARTSW2CONF_H
#define UARTSW2CONF_H

// constants/macros/typdefs

#define UARTSW_RX_BUFFER_SIZE	0x20		// UART receive buffer size in bytes

//#define UARTSW_INVERT						// define to invert polarity of RX/TX signals
// When non-inverted, the serial line is appropriate for passing through 
// an RS232 driver like the MAX232. When inverted, the serial line can
// directly drive/receive RS232 signals to/from DB9 connector. Be sure
// to use a current-limiting resistor and perhaps a diode-clamp circuit when
// connecting incoming RS232 signals to a micorprocessor I/O pin.

// if non-inverted, the serial line idles high (logic 1) between bytes
// if inverted, the serial line idles low (logic 0) between bytes

// UART transmit pin defines
#define UARTSW_TX_PORT			PORTD		// UART Transmit Port
#define	UARTSW_TX_DDR			DDRD		// UART	Transmit DDR
#define	UARTSW_TX_PIN			PD4			// UART	Transmit Pin

// UART receive pin defines
// This pin must correspond to the
// External Interrupt 1 (INT1) pin for your process
#define UARTSW_RX_PORT			PORTD		// UART Receive Port
#define UARTSW_RX_DDR			DDRD		// UART	Receive DDR
#define UARTSW_RX_PORTIN		PIND		// UART Receive Port Input
#define UARTSW_RX_PIN			PD3			// UART Receive Pin

#endif
