// This code allows you to drive up to 8 RC servos from any combination of
// ports and pins on the AVR processor. Using interrupts, this code continuously
// sends control signals to the servo to maintain position even while your code
// is doing other work.
//
// The servoInit and servoOff effectively turn on and turn off servo control.
// When you run ServoInit, it automatically assignes each "channel" of servo
// control to be output on the SERVO_DEFAULT_PORT. One "channel" of servo
// control can control one servo and must be assigned single I/O pin for output.
//
// If your're using all eight channels (SERVO_NUM_CHANNELS = 8), then by default
// the code will use SERVO_DEFAULT_PORT pins 0-7.
// If you're only using four channels, then pins 0-3 will be used by default.
//
// The command servoSetChannelIO(channel, port, pin) allows you to reassign the
// output of any channel to any port and I/O pin you choose. For example, if you
// have an RC servo connected to PORTC, pin6, and you wish to use channel 2
// to control it, use:
//
// 		servoSetChannelIO(2, _SFR_IO_ADDR(PORTC), 6)
//
// 		(NOTE: you must include the "_SFR_IO_ADDR()" part around your port)
//
// The servoSetPosition and servoGetPosition commands allow you to command
// a given servo to your desired position. The position you request must
// lie between the SERVO_MIN and SERVO_MAX limit you defined.
//
// WARNING: This servo library has been tested to work without issue on several
// different AVR processors and with serveral different brands/kinds of servos.
// However:
// 	- Proper output duty cycles are dependent upon a user calibration and
// 	  configuration
// 	- IF YOUR SERVOS ARE EXCEPTIONALLY POWERFUL, AN ERRONEOUS OUTPUT DUTY
// 	  CYCLE GENERATED FROM THIS CODE OR ANY OTHER SOURCE CAN DAMAGE YOUR
// 	  SERVO'S INTERNAL DRIVER CHIP OR ITS GEARS!
// 	- I have never experienced any servo damage from erroneous control signal
// 	  input, but it is possible.

#ifndef SERVO_H
#define SERVO_H

#include <inttypes.h>
#include "global.h"
#include "timer.h"

// include configuration
#include "servoconf.h"

typedef struct struct_ServoChannel
{
	// hardware I/O port and pin for this channel
	uint8_t port;
	uint8_t pin;
	// PWM duty setting which corresponds to servo position
	uint16_t duty;
} ServoChannelType;

// functions

// initializes servo system
// 		You must run this to begin servo control
void servoInit(void);

// turns off servo system
// 		This stops controlling the servos and
// 		returns control of the SERVOPORT to your code
void servoOff(void);

// set the port and I/O pin you wish to use for a given channel
// 		If you do not assign a port and I/O pin for a channel (ie. you don't
// 		use this command) then all output will be done through the
// 		SERVEO_DEFAULT_PORT. See above definition of SERVO_DEFAULT_PORT.
void servoSetChannelIO(uint8_t channel, uint8_t port, uint8_t pin);

// set and get servo position on a given channel
// 	servoSetPosition() commands the servo on <channel> to the position you
// 		desire. The position input must lie between 0 and POSITION_MAX and
// 		will be automatically scaled to raw positions between SERVO_MIN and
// 		SERVO_MAX
//	servoGetPosition() returns the most recently set position of the servo on
//		<channel>. The return value will be scaled 0->POSITION_MAX
void servoSetPosition(uint8_t channel, uint8_t position);
uint8_t servoGetPosition(uint8_t channel);

//	set and get raw servo position on a given channel
//		Works like non-raw commands but position is not scaled. Position must
//		be between SERVO_MIN and SERVO_MAX
void servoSetPositionRaw(uint8_t channel, uint16_t position);
uint16_t servoGetPositionRaw(uint8_t channel);

//	servo interrupt service routine
void servoService(void);

#endif
