#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c.h"

//#define I2C_DEBUG
#ifdef I2C_DEBUG
#include "rprintf.h"
#include "uart.h"
#endif

// Standard I2C bit rate are:
// 100KHz for slow speed
// 400KHz for high speed

// I2C state and address variables
static volatile eI2cStateType I2cState;
static uint8_t I2cDeviceAddrRW;
// send/transmit buffer (outgoing data)
static uint8_t I2cSendData[I2C_SEND_DATA_BUFFER_SIZE];
static uint8_t I2cSendDataIndex;
static uint8_t I2cSendDataLength;
// receive buffer (incoming data)
static uint8_t I2cReceiveData[I2C_RECEIVE_DATA_BUFFER_SIZE];
static uint8_t I2cReceiveDataIndex;
static uint8_t I2cReceiveDataLength;

// function pointer to i2c receive routine
// I2cSlaveReceive is called when this processor
// is addressed as a slave for writing
static void (*i2cSlaveReceive)(uint8_t receiveDataLength, uint8_t* receiveData);
// I2cSlaveTransmit is called when this processor
// is addressed as a slave for reading
static uint8_t (*i2cSlaveTransmit)(uint8_t transmitDataLengthMax, uint8_t* transmitData);

// functions
void i2cInit(void)
{
	// set pull-up resistors on I2C bus pins
	sbi(I2C_SCL_PORT, I2C_SCL_PIN);
	sbi(I2C_SDA_PORT, I2C_SDA_PIN);

	// clear SlaveReceive and SlaveTrasmit handler to null
	i2cSlaveReceive = 0;
	i2cSlaveTransmit = 0;
	// set i2c bit rate to 100KHz
	i2cSetBitrate(100);
	// enable TWI (two-wire interface)
	sbi(TWCR, TWEN);
	// set state
	I2cState = I2C_IDLE;
	// enable TWI interrupt and slave address ACK
	sbi(TWCR, TWIE);
	sbi(TWCR, TWEA);
	// enable interrupts
	sei();
}

void i2cSetBitrate(uint16_t bitrateKHz)
{
	uint8_t bitrate_div;
	// set i2c bitrate
	// SCL freq = F_CPU/(16+2*TWBR))
	bitrate_div = ((F_CPU/1000l)/bitrateKHz);
	if (bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;
	outb(TWBR, bitrate_div);
}

void i2cSetLocalDeviceAddr(uint8_t deviceAddr, uint8_t genCallEn)
{
	// set local device address (used in slave mode only)
	outb(TWAR, ((deviceAddr&0xFE) | (genCallEn?1:0)));
}

void i2cSetSlaveReceiveHandler(void (*i2cSlaveRx_func)(uint8_t receiveDataLength, uint8_t* receiveData))
{
	i2cSlaveReceive = i2cSlaveRx_func;
}

void i2cSetSlaveTransmitHandler(uint8_t (*i2cSlaveTx_func)(uint8_t transmitDataLengthMax, uint8_t* transmitData))
{
	i2cSlaveTransmit = i2cSlaveTx_func;
}

inline void i2cSendStart(void)
{
	// send start condition
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTA));
}

inline void i2cSendStop(void)
{
	// transmit stop condition
	// leave with TWEA on for salve receiving
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA)|BV(TWSTO));
}

inline void i2cWaitForComplete(void)
{
	// wait for i2c interface to complete operation
	while ( !(inb(TWCR) & BV(TWINT)) );
}

inline void i2cSendByte(uint8_t data)
{
	// save data to the TWDR
	outb(TWDR, data);
	// begin send
	outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
}

inline void i2cReceiveByte(uint8_t ackFlag)
{
	// begin receive over i2c
	if ( ackFlag )
	{
		// ackFlag = TRUE: ACK the received data
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
	}
	else
	{
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
	}
}

inline uint8_t i2cGetReceivedByte(void)
{
	// retieve received data byte from i2c TWDR
	return ( inb(TWDR) );
}

inline uint8_t i2cGetStatus(void)
{
	// retieve current i2c status from i2c TWSR
	return ( inb(TWSR) );
}

void i2cMasterSend(uint8_t deviceAddr, uint8_t length, uint8_t* data)
{
	uint8_t i;
	// wait for interface to be ready
	while (I2cState);
	// set state
	I2cState = I2C_MASTER_TX;
	// save data
	I2cDeviceAddrRW = (deviceAddr & 0xFE); // RW cleard: write operation
	for (i=0; i<length; i++)
		I2cSendData[i] = *data++;
	I2cSendDataIndex = 0;
	I2cSendDataLength = length;
	// send start condition
	i2cSendStart();
}

void i2cMasterReceive(uint8_t deviceAddr, uint8_t length, uint8_t* data)
{
	uint8_t i;
	// wait for interface to be ready
	while (I2cState);
	// set state
	I2cState = I2C_MASTER_RX;
	// save data
	I2cDeviceAddrRW = (deviceAddr|0x01); // RW set: read operation
	I2cReceiveDataIndex = 0;
	I2cReceiveDataLength = length;
	// send start condition
	i2cSendStart();
	// wait for data
	while (I2cState);
	// return data
	for (i=0; i< length; i++)
		*data++ = I2cReceiveData[i];
}

uint8_t i2cMasterSendNI(uint8_t deviceAddr, uint8_t length, uint8_t* data)
{
	uint8_t retval = I2C_OK;

	// disable TWI interrupt
	cbi(TWCR, TWIE);

	// send start condition
	i2cSendStart();
	i2cWaitForComplete();

	// send device address with write
	i2cSendByte( deviceAddr & 0xFE );
	i2cWaitForComplete();

	// check if device is present and live
	if (inb(TWSR) == TW_MT_SLA_ACK)
	{
		// send data
		while (length)
		{
			i2cSendByte(*data++);
			i2cWaitForComplete();
			length--;
		}
	}
	else
	{
		// device did not ACK it's address,
		// data will not be transferred
		// return error
		retval = I2C_ERROR_NODEV;
	}

	// transmit stop condition
	// leave with TWEA on for slave receiving
	i2cSendStop();

	// enable TWI interrupt
	sbi(TWCR, TWIE);

	return retval;
}

uint8_t i2cMasterReceiveNI(uint8_t deviceAddr, uint8_t length, uint8_t *data)
{
	uint8_t retval = I2C_OK;

	// disable TWI interrupt
	cbi(TWCR, TWIE);

	// send start condition
	i2cSendStart();
	i2cWaitForComplete();

	// send device address with read
	i2cSendByte(deviceAddr | 0x01);
	i2cWaitForComplete();

	//rprintf("receive TWSR = 0x%x\n", TWSR);
	// check if device is present and live
	if (inb(TWSR) == TW_MR_SLA_ACK)
	{
		// accept receive data and ack it
		while(length > 1)
		{
			i2cReceiveByte(TRUE);
			i2cWaitForComplete();
			*data++ = i2cGetReceivedByte();
			// decrement length
			length--;
		} 

		// accept receive data and nack it (last-byte signal)
		i2cReceiveByte(FALSE);
		i2cWaitForComplete();
		*data++ = i2cGetReceivedByte();
	} 
	else
	{
		// device did not ACK it's address,
		// data will not be transferred
		// return error
		retval = I2C_ERROR_NODEV;
	}

	// transmit stop condition
	// leave with TWEA on for slave receiving
	i2cSendStop();

	// enable TWI interrupt
	sbi(TWCR, TWIE);

	return retval;
}

// I2C (TWI) interrupt service routine
ISR(TWI_vect)
{
	// read status bits
	uint8_t status = inb(TWSR) & TWSR_STATUS_MASK;

	switch (status)
	{
	// Master General
	case TW_START:				// 0x08: Sent start condition	
	case TW_REP_START:			// 0x10: Sent repeated start condition
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: M->START\r\n");
		rprintfInit(uartSendByte);
		#endif
		// send device address
		i2cSendByte(I2cDeviceAddrRW);
		break;

	// Master Transmitter & Receiver status codes
	case TW_MT_SLA_ACK:			// 0x18: Slave address acknowledged
	case TW_MT_DATA_ACK:		// 0x28: Data acknowledged
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MT->SLA_ACK or DATA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		if (I2cSendDataIndex < I2cSendDataLength)
		{
			// send data
			i2cSendByte(I2cSendData[I2cSendDataIndex++]);
		}
		else
		{
			// transmit stop condition, enable SLA ACK
			i2cSendStop();
			// set state
			I2cState = I2C_IDLE;
		}
		break;
	case TW_MR_DATA_NACK:		// 0x58: Data received, NACK reply issue
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MR->DATA_NACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// store final received data tye
		I2cReceiveData[I2cReceiveDataIndex++] = inb(TWDR);
		// continue to transmit STOP condition
	case TW_MR_SLA_NACK:		// 0x48: Slave address not acknowleged
	case TW_MT_SLA_NACK:		// 0x20: Slave address not acknowleged
	case TW_MT_DATA_NACK:		// 0x30: Data not acknowledged
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MTR->SLA_NACK or MT->DATA_NACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// transmit stop condition, enable SLA ACK
		i2cSendStop();
		// set state
		I2cState = I2C_IDLE;
		break;
	case TW_MT_ARB_LOST:		// 0x38: Bus arbitration lost
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MT->ARB_LOST\r\n");
		rprintfInit(uartSendByte);
		#endif
		// release bus
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
		// set state
		I2cState = I2C_IDLE;
		break;
	case TW_MR_DATA_ACK:		// 0x50: Data acknowledged
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MR->DATA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// store received data byte
		I2cReceiveData[I2cReceiveDataIndex++] = inb(TWDR);
		// fall-through to see if more bytes will be received
	case TW_MR_SLA_ACK:			// 0x40: Slave address acknowledged
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: MR->SLA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		if (I2cReceiveDataIndex < (I2cReceiveDataLength-1))
			// data byte will be received, replay with ACK (more bytes in transfer)
			i2cReceiveByte(TRUE);
		else
			// data byte will be received, replay with NACK (final byte in transfer)
			i2cReceiveByte(FALSE);
		break;

	// Slave Receiver status codes
	case TW_SR_SLA_ACK:			// 0x60: own SLA+W has been received, ACK has breen returned
	case TW_SR_ARB_LOST_SLA_ACK:	// 0x68: own SLA+W has been received, ACK has been returned
	case TW_SR_GCALL_ACK:		// 0x70: GCA+W has been received, ACK has been returned
	case TW_SR_ARB_LOST_GCALL_ACK: // 0x78: GCA+W has been received, ACK has been returned
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: SR->SLA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// we are being addressed as slave for writing (data will be received from master)
		// set state
		I2cState = I2C_SLAVE_RX;
		// prepare buffer
		I2cReceiveDataIndex = 0;
		// receive data byte and return ACK
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		break;
	case TW_SR_DATA_ACK:		// 0x80: data byte has been received, ACK has been returned
	case TW_SR_GCALL_DATA_ACK:	// 0x90: data byte has been received, ACK has been returned
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: SR->DATA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// get previously received data byte
		I2cReceiveData[I2cReceiveDataIndex++] = inb(TWDR);
		// check receive buffer status
		if (I2cReceiveDataIndex < I2C_RECEIVE_DATA_BUFFER_SIZE)
		{
			// receive data byte and return ACK
			i2cReceiveByte(TRUE);
		}
		else
		{
			// receive data byte and return NACK
			i2cReceiveByte(FALSE);
		}
		break;
	case TW_SR_DATA_NACK:		// 0x88: data byte has been received, NACK has been returned
	case TW_SR_GCALL_DATA_NACK:	// 0x98: data byte has been received, NACK has been returned
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: SR->DATA_NACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// receive data byte and return NACK
		i2cReceiveByte(FALSE);
		break;
	case TW_SR_STOP:			// 0xA0: STOP or REPEATED START has been received while addressed as slave
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: SR->SR_STOP\r\n");
		rprintfInit(uartSendByte);
		#endif
		// switch to SR mode with SLA ACK
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		// i2c receive is complete, call i2cSlaveReceive
		if (i2cSlaveReceive) i2cSlaveReceive(I2cReceiveDataIndex, I2cReceiveData);
		// set state
		I2cState = I2C_IDLE;
		break;

	// Slave Transmitter
	case TW_ST_SLA_ACK:			// 0xA8: own SLA+R has been received, ACK has been returned
	case TW_ST_ARB_LOST_SLA_ACK:// 0xB0: GCA+R has been received, ACK has been returned
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: ST->SLA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// we are being addressed as slave for reading (data must be transmitted back to master)
		// set state
		I2cState = I2C_SLAVE_TX;
		// request data from application
		if (i2cSlaveTransmit) I2cSendDataLength = i2cSlaveTransmit(I2C_SEND_DATA_BUFFER_SIZE, I2cSendData);
		// reset data index
		I2cSendDataIndex = 0;
		// fall-through to transmit first data byte
	case TW_ST_DATA_ACK:		// 0xB8: data byte has been transmitted, ACK has been received
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: ST->DATA_ACK\r\n");
		rprintfInit(uartSendByte);
		#endif
		// transmit data byte
		outb(TWDR, I2cSendData[I2cSendDataIndex++]);
		if (I2cSendDataIndex < I2cSendDataLength)
			// expect ACK to data byte
			outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		else
			// expect NACK to data byte
			outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
		break;
	case TW_ST_DATA_NACK:		// 0xC0: data byte has been transmitted, NACK has been received
	case TW_ST_LAST_DATA:		// 0xC8:
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: ST->DATA_NACK or LAST_DATA\r\n");
		rprintfInit(uartSendByte);
		#endif
		// all done
		// switch to open slave
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
		// set state
		I2cState = I2C_IDLE;
		break;

	// Misc
	case TW_NO_INFO:			// 0xF8: No relevant state information
		// do nothing
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: NO_INFO\r\n");
		rprintfInit(uartSendByte);
		#endif
		break;
	case TW_BUS_ERROR:			// 0x00: Bus error due to illegal start or stop condition
		#ifdef I2C_DEBUG
		rprintfInit((void (*)(unsigned char))uartAddToTxBuffer);
		rprintf("I2C: BUS_ERROR\r\n");
		rprintfInit(uartSendByte);
		#endif
		// reset internal hardware and release bus
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWSTO)|BV(TWEA));
		// set state
		I2cState = I2C_IDLE;
		break;
	}
}

eI2cStateType i2cGetState(void)
{
	return I2cState;
}
