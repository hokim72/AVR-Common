#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include "global.h"
#include "timer.h"

// Program ROM constants
// the prescale division values store in order of timer control register index
// STOP, CLK, CLK/8, CLK/64, CLK/256, CLK/1024
const unsigned short PROGMEM TimerPrescaleFactor[] = {0, 1, 8, 64, 256, 1024};

// Global variables
// time registers
volatile unsigned long TimerPauseReg;
volatile unsigned long Timer0Reg0;
volatile unsigned long Timer2Reg0;

typedef void (*voidFuncPtr)(void);
volatile static voidFuncPtr TimerIntFunc[TIMER_NUM_INTERRUPTS];

// delay for a minimum of <us> microseconds
// the time resolution is dependent on the time the loop takes
// e.g. with 4Mhz and 5 cycles per loop, the resolution is 1.25 us
void delay_us(unsigned short time_us)
{
	unsigned short delay_loops;
	register unsigned short i;

	delay_loops = (time_us+3)/5*CYCLES_PER_US; // +3 for rounding up (dirty)

	// one loop takes 5 cpu cycles
	for (i=0; i<delay_loops; i++) {};
}

void timerInit(void)
{
	uint8_t intNum;
	// detach all user functions from interrupts
	for (intNum=0; intNum<TIMER_NUM_INTERRUPTS; intNum++)
		timerDetach(intNum);

	// initialize all timers
	timer0Init();
	timer1Init();
	timer2Init();

	// enable interrupts
	sei();
}

void timer0Init()
{
	// initialize timer 0
	timer0SetPrescaler( TIMER0PRESCALE );	// set prescaler
	outb(TCNT0, 0);							// reset TCNT0
	sbi(TIMSK0, TOIE0);						// enable TCNT0 overflow interrupt

	timer0ClearOverflowCount();				// initialize time registers
}

void timer1Init()
{
	// initialize timer 1
	timer1SetPrescaler( TIMER1PRESCALE );	// set prescaler
	outb(TCNT1H, 0);						// reset TCNT1
	outb(TCNT1L, 0);
	sbi(TIMSK1, TOIE1);						// enable TCNT1 overflow interrupt
}

void timer2Init(void)
{
	// initialize timer 2
	timer2SetPrescaler( TIMER2PRESCALE );	// set prescaler
	outb(TCNT2, 0);							// reset TCNT2
	sbi(TIMSK2, TOIE2);						// enable TCNT2 overflow

	timer2ClearOverflowCount();				// initialize time registers
}

void timer0SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 0
	outb(TCCR0B, (inb(TCCR0B) & ~TIMER_PRESCALE_MASK) | prescale);
}

void timer1SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 1
	outb(TCCR1B, (inb(TCCR1B) & ~TIMER_PRESCALE_MASK) | prescale);
}

void timer2SetPrescaler(uint8_t prescale)
{
	// set prescaler on timer 2
	outb(TCCR2B, (inb(TCCR2B) & ~TIMER_PRESCALE_MASK) | prescale);
}

uint16_t timer0GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(inb(TCCR0B) & TIMER_PRESCALE_MASK)));
}

uint16_t timer1GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(inb(TCCR1B) & TIMER_PRESCALE_MASK)));
}

uint16_t timer2GetPrescaler(void)
{
	// get the current prescaler setting
	return (pgm_read_word(TimerPrescaleFactor+(inb(TCCR2B) & TIMER_PRESCALE_MASK)));
}

void timerAttach(uint8_t interruptNum, void (*userFunc)(void))
{
	// make sure the interrupt number is within bounds
	if (interruptNum < TIMER_NUM_INTERRUPTS)
	{
		// set the interrupt function to run
		// the supplied user's function
		TimerIntFunc[interruptNum] = userFunc;
	}
}

void timerDetach(uint8_t interruptNum)
{
	// make sure the interrupt number is within bounds
	if (interruptNum < TIMER_NUM_INTERRUPTS)
	{
		// set the interrupt function to run noting
		TimerIntFunc[interruptNum] = 0;
	}
}

void timerPause(unsigned short pause_ms)
{
	// pauses for exactly <pause_ms> number of milliseconds
	uint8_t timerThres;
	uint32_t ticRateHz;
	uint32_t pause;

	// capture current pause timer value
	timerThres = inb(TCNT0);
	// reset pause timer overflow count
	TimerPauseReg = 0;
	// calculate delay for [pause_ms] milliseconds
	// prescaler division = 1<<(pgm_read_byte(TimerPrescaleFactor+inb(TCCR0B)))
	ticRateHz = F_CPU/timer0GetPrescaler();
	// precision management
	// prevent overflow and precision underflow
	// - could and more conditions to improve accuracy
	if ( ((ticRateHz < 429497) && (pause_ms <= 10000)) )
		pause = (pause_ms*ticRateHz)/1000;
	else
		pause = pause_ms*(ticRateHz/1000);
	
	// loop until time expires
	while ( ((TimerPauseReg<<8) | inb(TCNT0)) < (pause+timerThres) )
	{
		if (TimerPauseReg < (pause>>8));
		{
			// save power by idling the processor
			set_sleep_mode(SLEEP_MODE_IDLE);
			sleep_mode();
		}
	}
}

void timer0ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer0Reg0 = 0; // initialize time registers
}

long timer0GetOverflowCount(void)
{
	// return the current timer overflow count
	//  (this is since the last timer0ClearOverflowCount() command was called)
	return Timer0Reg0;
}

void timer2ClearOverflowCount(void)
{
	// clear the timer overflow counter registers
	Timer2Reg0 = 0; // initialize time registers
}

long timer2GetOverflowCount(void)
{
	// return the current timer overflow count
	// (this is since the last timer2ClearOverflowCount() command was called)
	return Timer2Reg0;
}

void timer1PWMInit(uint8_t bitRes)
{
	// configure timer1 for use with PWM output
	// on OC1A and OC1B pins

	// enable timer1 as 8,9,10bit PWM
	cbi(TCCR1B, WGM13);
	if (bitRes == 9)
	{
		// 9bit mode
		sbi(TCCR1B, WGM12);
		sbi(TCCR1A, WGM11);
		cbi(TCCR1A, WGM10);
	} 
	else if (bitRes == 10)
	{
		// 10bit mode
		sbi(TCCR1B, WGM12);
		sbi(TCCR1A, WGM11);
		sbi(TCCR1A, WGM10);
	}
	else
	{
		// default 8bit mode
		sbi(TCCR1B, WGM12);
		cbi(TCCR1A, WGM11);
		sbi(TCCR1A, WGM10);
	}

	// clear output compare value A
	outb(OCR1AH, 0);
	outb(OCR1AL, 0);
	// clear output compare value B
	outb(OCR1BH, 0);
	outb(OCR1BL, 0);
}

void timer1PWMInitICR(uint16_t topcount)
{
	// set PWM mode with ICR top-count
	sbi(TCCR1B, WGM13);
	sbi(TCCR1B, WGM12);
	sbi(TCCR1A, WGM11);
	cbi(TCCR1A, WGM10);

	// set top count value
	ICR1 = topcount;

	// clear output compare value A
	OCR1A = 0;
	// clear output compare value B
	OCR1B = 0;
}

void timer1PWMOff(void)
{
	// turn off timer1 PWM mode
	cbi(TCCR1B, WGM13);
	cbi(TCCR1B, WGM12);
	cbi(TCCR1A, WGM11);
	cbi(TCCR1A, WGM10);

	// set PWM1A/B (OutputCompare action) to none
	timer1PWMAOff();
	timer1PWMBOff();
}

void timer1PWMAOn(void)
{
	// turn on channel A (OC1A) PWM out
	// set OC1A as non-inverted PWM
	sbi(TCCR1A, COM1A1);
	cbi(TCCR1A, COM1A0);
}

void timer1PWMBOn(void)
{
	// turn on channel B (OC1B) PWM out
	// set OC1B as non-inverted PWM
	sbi(TCCR1A, COM1B1);
	cbi(TCCR1A, COM1B0);
}

void timer1PWMAOff(void)
{
	// turn off channel A (OC1A) PWM output
	// set OC1A (OutputCompare action) to none
	cbi(TCCR1A, COM1A1);
	cbi(TCCR1A, COM1A0);
}

void timer1PWMBOff(void)
{
	// turn off channel B (OC1B) PWM output
	// set OC1B (OutputCompare action) to none
	cbi(TCCR1A, COM1B1);
	cbi(TCCR1A, COM1B0);
}

void timer1PWMASet(uint16_t pwmDuty)
{
	// set PWM (output compare) duty for channel A
	// this PWM output is generated on OC1A pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	// 			pwmDuty should be in the range 0-511 for 9bit PWM
	// 			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR1A = pwmDuty;
}

void timer1PWMBSet(uint16_t pwmDuty)
{
	// set PWM (output compare) duty for channel B
	// this PWM output is generated on OC1B pin
	// NOTE:	pwmDuty should be in the range 0-255 for 8bit PWM
	// 			pwmDuty should be in the range 0-511 for 9bit PWM
	// 			pwmDuty should be in the range 0-1023 for 10bit PWM
	OCR1B = pwmDuty;
}

ISR(TIMER0_OVF_vect)
{
	Timer0Reg0++;		// increment low-order counter

	// increment pause counter
	TimerPauseReg++;

	// if a user function is defined, execute it 
	if (TimerIntFunc[TIMER0OVERFLOW_INT])
		TimerIntFunc[TIMER0OVERFLOW_INT]();
}

ISR(TIMER1_OVF_vect)
{
	// if a user function is defined, execute it 
	if (TimerIntFunc[TIMER1OVERFLOW_INT])
		TimerIntFunc[TIMER1OVERFLOW_INT]();
}

ISR(TIMER2_OVF_vect)
{
	Timer2Reg0++;		// increment low-order counter

	// if a user function is defined, execute it 
	if (TimerIntFunc[TIMER2OVERFLOW_INT])
		TimerIntFunc[TIMER2OVERFLOW_INT]();
}

// Interrupt handler for OutputCompare0A match (OC0A) interrupt
ISR(TIMER0_COMPA_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER0OUTCOMPAREA_INT])
		TimerIntFunc[TIMER0OUTCOMPAREA_INT]();
}

// Interrupt handler for OutputCompare0B match (OC0B) interrupt
ISR(TIMER0_COMPB_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER0OUTCOMPAREB_INT])
		TimerIntFunc[TIMER0OUTCOMPAREB_INT]();
}

// Interrupt handler for OutputCompare1A match (OC1A) interrupt
ISR(TIMER1_COMPA_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER1OUTCOMPAREA_INT])
		TimerIntFunc[TIMER1OUTCOMPAREA_INT]();
}

// Interrupt handler for OutputCompare1B match (OC1B) interrupt
ISR(TIMER1_COMPB_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER1OUTCOMPAREB_INT])
		TimerIntFunc[TIMER1OUTCOMPAREB_INT]();
}

// Interrupt handler for OutputCompare2A match (OC2A) interrupt
ISR(TIMER2_COMPA_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER2OUTCOMPAREA_INT])
		TimerIntFunc[TIMER2OUTCOMPAREA_INT]();
}

// Interrupt handler for OutputCompare2B match (OC2B) interrupt
ISR(TIMER2_COMPB_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER2OUTCOMPAREB_INT])
		TimerIntFunc[TIMER2OUTCOMPAREB_INT]();
}

// Interrupt handler for InputCapture1 (IC1) interrupt
ISR(TIMER1_CAPT_vect)
{
	// if a user function is defined, execute it
	if (TimerIntFunc[TIMER1INPUTCAPTURE_INT])
		TimerIntFunc[TIMER1INPUTCAPTURE_INT]();
}
