// This library is designed to facilitate the output of square wave pulses
// at a frequency determinded by the user. The user may specify a continuous
// stream of pulses, or a certain fixed number. Common uses include stepper
// motor speed control, tone generation, communications, etc. The library uses
// the AVR process built-in timers and pulse output is on the timer Output
// Compare (OC) pins. This library requires the timer function library to work.
//
// The allowable range of frequencies which can be generated is governed
// by the tic rate of the timer (therefore the CPU clock rate and the timer
// prescaler), and the computing speed of the processor itself. 
// See the SetFreq commands for more details.
//
// In order for the pulse library to work, pulseInit() will attach the pulse
// service routines to the timer interrupts using the timerAttach function.
// You must not detach the service routines during pulse library operation.

#ifndef PULSE_H
#define PULSE_H

#include <inttypes.h>
#include "global.h"

// functions

// Master Pulse Commands
// pulseInit()
// 		Initializes the pulse system/library.
void pulseInit(void);

// Pulse commands for timer1
// pulseT1Init()
// 		configures the timer1 hardware to produce pulses on pins OC1A and OC1B.
// 		A "pulse" is considered to be one high and low period of a square wave.
void pulseT1Init(void);

// pulseT1Off()
// 		Turns pulse output off immediately (cancels running pulse operations).
// 		Unconfigures hardware and interrupts.
void pulseT1Off(void);

// pulseT1ASetFreq() and pulseT1BSetFreq()
// 		sets the frequency in hertz for each channel of square-wave pulse output
// 		Note1: the frequency <freqHz> must always be greater than zero
// 		Note2: both channels share the same frequency range which is governed
// 		by the timer1 prescaler setting. A prescaler setting of DIV/8 allows
// 		frequencies of a few hertz through a few kilohertz.
//
// 		Lower frequency bound = overflow rate of timer1 at current prescaling
// 		Upper frequency bound - the tics rate of timer1 at current prescaling,
// 								or approx. the (clock rate of the processor)/50,
// 								whichever is smaller
void pulseT1ASetFreq(uint16_t freqHz);
void pulseT1BSetFreq(uint16_t freqHz);

// pulseT1ARun() and pulseT1BRun();
// 		Sets the number of square-wave pulses to be output on the given channel.
// 		For continuous (unlimited) pulse output, use nPulses = 0. Pulses begin
// 		coming out immediately.
// 		Note: <nPulses> must be between 0 and 32767
void pulseT1ARun(uint16_t nPulses);
void pulseT1BRun(uint16_t nPulses);

// pulseT1AStop() and pulseT1BStop();
// 		Stop pulse output at the next cycle (regardless of the number of
// 		remaining pulses).
void pulseT1AStop(void);
void pulseT1BStop(void);

// pulseT1ARemaining() and pulseT1BRemaining()
// 		Returns the number of pulses remaining to be output for each channel.
// 		This function is useful for figuring out if the pulses are done.
uint16_t pulseT1ARemaining(void);
uint16_t pulseT1BRemaining(void);

// pulseT1AService() and pulseT1BService()
// 		Interrupt service routines for pulse output (do not call these functions
// 		directly)
void pulseT1AService(void);
void pulseT1BService(void);

#endif
