// This library provides an easy interface to the analog-to-digital converter
// available on many AVR processors.

#ifndef A2D_H
#define A2D_H

// defines

// A2D clock prescaler select
// 		*selects how much the CPU clock frequency is divided
// 		to create the A2D clock frequency
// 		*lower division ratios make conversion go faster
// 		*higher division ratios make conversions more accurate
#define ADC_PRESCALE_DIV2		0x00	// 0x01, 0x00 -> CPU clk/2
#define ADC_PRESCALE_DIV4		0x02	// 0x02 -> CPU clk/4
#define ADC_PRESCALE_DIV8		0x03	// 0x03 -> CPU clk/8
#define ADC_PRESCALE_DIV16		0x04	// 0x04 -> CPU clk/16
#define ADC_PRESCALE_DIV32		0x05	// 0x05 -> CPU clk/32
#define ADC_PRESCALE_DIV64		0x06	// 0x06 -> CPU clk/64
#define ADC_PRESCALE_DIV128		0x07	// 0x07 -> CPU clk/128
// default value
#define ADC_PRESCALE			ADC_PRESCALE_DIV64
// do not change the mask value
#define ADC_PRESCALE_MASK		0x07

// A2D voltage reference select
//		*this determines what is used as the
//		full-scale voltage point for A2D conversions
#define	ADC_REFERENCE_AREF		0x00	// 0x00 -> AREF pin, internal VREF turned off
#define	ADC_REFERENCE_AVCC		0x01	// 0x01 -> AVCC pin, internal VREF turned off
#define	ADC_REFERENCE_RSVD		0x02	// 0x02 -> Reserved
#define ADC_REFERENCE_11V		0x03	// 0x03 -> Internal 1.1 VREF
// default value
#define ADC_REFERENCE			ADC_REFERENCE_AVCC
// do not change the mask value
#define ADC_REFERENCE_MASK		0xC0

// bit mask for A2D channel multiplexer
#define ADC_MUX_MASK			0x0F

// channel defines (for reference and use in code)
// these channels supported by all AVRs with A2D
#define ADC_CH_ADC0				0x00
#define ADC_CH_ADC1				0x01
#define ADC_CH_ADC2				0x02
#define ADC_CH_ADC3				0x03
#define ADC_CH_ADC4				0x04
#define ADC_CH_ADC5				0x05
#define ADC_CH_ADC6				0x06
#define ADC_CH_ADC7				0x07
#define ADC_CH_ADC8				0x08
#define ADC_CH_11V				0x0E	// <1.1V voltage reference
#define ADC_CH_AGND				0x0F	// AGND

// function prototypes

// Initializes the A/D converter.
// Turns ADC on and prepares it for use
void a2dInit(void);

// Turn off A/D converter
void a2dOff(void);

// Sets the division ratio of the A/D converter clock.
// This function is automatically called from a2dInit()
// with a default value.
void a2dSetPrescaler(unsigned char prescale);

// Configures which voltage reference the A/D converter uses.
// This function is automatically called from a2dInit()
// with a default value.
void a2dSetReference(unsigned char ref);

// sets the a2d input channel
void a2dSetChannel(unsigned char ch);

// start a conversion on the current a2d input channel
void a2dStartConvert(void);

// return TRUE if conversion is complete
uint8_t a2IsComplete(void);

// Starts a conversion on A/D channel# ch,
// return the 10-bit value of the conversion when it is finished.
unsigned short a2dConvert10bit(unsigned char ch);

// Starts a conversion on A/D channel# ch,
// returns the 8-bit value of conversion when it is finished.
unsigned char a2dConvert8bit(unsigned char ch);

#endif
