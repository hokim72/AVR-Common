#include "uart.h"
#include "rprintf.h"
#include "i2c.h"
#include <util/delay.h>
#include <avr/power.h>

#define LM75_ADDRESS	0b10010000
#define LM75_TEMP_REGISTER	0b00000000

//#define DEBUG_LOW_LEVEL

int main(void)
{
	clock_prescale_set(clock_div_2);
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	i2cInit();
	#ifdef DEBUG_LOW_LEVEL
	cbi(TWCR, TWIE);
	#endif

	// print welcom message
	//rprintfProgStrM("\r\n====  i2c Thermometer  ====\r\n");

	uint8_t buf[2];
	while (1) {


	#ifdef DEBUG_LOW_LEVEL
		i2cSendStart();
		i2cWaitForComplete();
		rprintf("send start : 0x%x\n", TWSR); //0x8 : start condition transmitted
		i2cSendByte((LM75_ADDRESS|0x00));
		i2cWaitForComplete();
		rprintf("send address : 0x%x\n", TWSR); //0x18 : SLA+W transmitted, ACK received
		i2cSendByte(LM75_TEMP_REGISTER);
		i2cWaitForComplete();
		rprintf("send byte : 0x%x\n", TWSR); //0x28 : data transmitted, ACK received
		i2cSendStop();
		rprintf("send stop : 0x%x\n", TWSR); //0xf8 : no state information available

		i2cSendStart();
		i2cWaitForComplete();
		rprintf("send start : 0x%x\n", TWSR); //0x08 : start condition transmitted
		i2cSendByte((LM75_ADDRESS|0x01));
		i2cWaitForComplete();
		rprintf("receive address : 0x%x\n", TWSR); //0x40 :  SLA+R transmitted, ACK received

		i2cReceiveByte(TRUE);
		i2cWaitForComplete();
		buf[0] = i2cGetReceivedByte();
		rprintf("1st received byte : 0x%x\n", TWSR); //0x50 : data received, ACK returned

		i2cReceiveByte(FALSE);
		i2cWaitForComplete();
		buf[1] = i2cGetReceivedByte();
		rprintf("2nd received byte : 0x%x\n", TWSR); //0x58 : data received, NACK returned

		i2cSendStop();
		rprintf("send stop : 0x%x\n", TWSR); //0xf8 : no state information available
		#endif
	   
		buf[0] = LM75_TEMP_REGISTER;
		//i2cMasterSendNI(LM75_ADDRESS, 1, buf);
		//i2cMasterReceiveNI(LM75_ADDRESS, 2, buf);
		i2cMasterSend(LM75_ADDRESS, 1, buf);
		i2cMasterReceive(LM75_ADDRESS, 2, buf);

		//uartSendTxBuffer();


		rprintf("%d", buf[0]);
		if (buf[1] & BV(7))
			rprintf(".5");
		else
			rprintf(".0");
		rprintfCRLF();

	#ifdef DEBUG_LOW_LEVEL
		rprintf("==================================================\r\n");
	#endif
		_delay_ms(1000);
	}

	return 0;
}

