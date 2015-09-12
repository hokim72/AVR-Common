#include "uart.h"
#include "rprintf.h"
#include "timer.h"
#include "servo.h"

void servoTest(void);

int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// set the baud rate of the UART for our debug/reporting output
	uartSetBaudRate(9600);
	// set uartSendByte as the output for all rprintf statements
	rprintfInit(uartSendByte);
	// initialize the timer system
	timerInit();

	// print a little intro message so we know things are working
	rprintf("\r\nWelcom to Servo Test!\r\n");

	// being servo test
	servoTest();

	return 0;
}

void servoTest(void)
{
	uint8_t pos;
	uint8_t channel;

	// do some examples
	// initialize RC servo system
	servoInit();
	// setup servo output channel-to-I/Opint mapping
	// format is servoSetChannelIO(CHANNEL#, PORT, PIN);
	servoSetChannelIO(0, _SFR_IO_ADDR(PORTB), PB0);
	servoSetChannelIO(1, _SFR_IO_ADDR(PORTB), PB1);
	servoSetChannelIO(2, _SFR_IO_ADDR(PORTB), PB2);
	servoSetChannelIO(3, _SFR_IO_ADDR(PORTB), PB3);

	// set port pins to output
	outb(DDRB, 0x0F);

	pos = 0;

#define SPEED_SERVO 1

	// spin servos sequetially back and forth between their limits
	while(1)
	{
		for(channel=0; channel<SERVO_NUM_CHANNELS; channel++)
		{
			for(pos=0; pos<SERVO_POSITION_MAX; pos++)
			{
				servoSetPosition(channel, pos);
				timerPause(SPEED_SERVO);
			}
		}

		for(channel=0; channel<SERVO_NUM_CHANNELS; channel++)
		{
			for(pos=SERVO_POSITION_MAX; pos>=1; pos--)
			{
				servoSetPosition(channel, pos);
				timerPause(SPEED_SERVO);
			}
		}
	}
}
