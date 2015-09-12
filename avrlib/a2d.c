#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "a2d.h"

// global variables

// Software flag used to indicate when
// the a2d conversion is complete.
//volatile unsigned char a2dCompleteFlag;

// functions

// initialize a2d converter
void a2dInit(void)
{
	sbi(ADCSRA, ADEN);				// enable ADC (turn on ADC power)
	cbi(ADCSRA, ADATE);				// default to single sample convert mode
	a2dSetPrescaler(ADC_PRESCALE);	// set default prescaler
	a2dSetReference(ADC_REFERENCE);	// set default reference
	cbi(ADMUX, ADLAR);				// set to right-adjusted result

	//sbi(ADCSRA, ADIE);				// enable ADC interrupts

	//a2dCompleteFlag = FALSE;		// clear conversion complete flag
	sei();							// turn on interrupts (if not already on)
}

// turn off a2d converter
void a2dOff(void)
{
	//cbi(ADCSRA, ADIE);				// disable ADC interrupts
	cbi(ADCSRA, ADEN);				// disable ADC (turn off ADC power)
}

// configure A2D converter clock division (prescaling)
void a2dSetPrescaler(unsigned char prescale)
{
	outb(ADCSRA, ((inb(ADCSRA) & ~ADC_PRESCALE_MASK) | prescale));
}

// configure A2D converter voltage reference
void a2dSetReference(unsigned char ref)
{
	outb(ADMUX, ((inb(ADMUX) & ~ADC_REFERENCE_MASK) | (ref<<6)));
}

// sets the a2d input channel
void a2dSetChannel(unsigned char ch)
{
	outb(ADMUX, (inb(ADMUX) & ~ADC_MUX_MASK) | (ch & ADC_MUX_MASK)); // set channel
}

// start a conversion on the current a2d input channel
void a2dStartConvert(void)
{
	sbi(ADCSRA, ADIF);	// clear hardware "conversion complete" flag
	sbi(ADCSRA, ADSC);  // start conversion
}

// return TRUE if conversion is complete
uint8_t a2dIsComplete(void)
{
	return bit_is_set(ADCSRA, ADSC);
}

// Perform a 10-bit conversion
// starts conversion, waits until conversion is done, and return result
unsigned short a2dConvert10bit(unsigned char ch)
{
	//a2dCompleteFlag = FALSE;		// clear conversion complete flag
	outb(ADMUX, (inb(ADMUX) & ~ADC_MUX_MASK) | (ch & ADC_MUX_MASK)); // set channel
	sbi(ADCSRA, ADIF);				// clear hardware "conversion complete" flag
	sbi(ADCSRA, ADSC);				// start conversion

	while (bit_is_set(ADCSRA, ADSC) );	// wait until conversion complete

	// CAUTION: MUST READ ADCL BEFORE ADCH!!!
	return (inb(ADCL) | (inb(ADCH)<<8));	// read ADC (full 10 bits);
}

// Perfrom a 8-bit conversion
// starts conversion, waits until conversion is done, and returns result
unsigned char a2dConvert8bit(unsigned char ch)
{
	// do 10-bit conversion and return hightest 8bits
	return a2dConvert10bit(ch)>>2;	// return ADC MSB byte
}

// Interrupt handler for ADC complete interrupt.
//ISR(ADC_vect)
//{
	// set the a2d conversion flag to indicate "complete"
//	a2dCompleteFlag = TRUE;
//}
