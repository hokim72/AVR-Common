#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "timer.h"
#include "uartsw.h"

// uartsw transmit status and data variables
static volatile uint8_t UartswTxBusy;
static volatile uint8_t UartswTxData;
static volatile uint8_t UartswTxBitNum;

// baud rate common to transmit and receive
static volatile uint16_t UartswBaudRateDiv;

#ifndef UARTSW_TX_ONLY
// uartsw receive status and data variables
static volatile uint8_t UartswRxBusy;
static volatile uint8_t UartswRxData;
static volatile uint8_t UartswRxBitNum;
// receive buffer
static cBuffer uartswRxBuffer;				// uartsw receive buffer
// automatically allocate space in ram for each buffer
static unsigned char uartswRxData[UARTSW_RX_BUFFER_SIZE];
#endif

// functions

// enable and initialize the software uart
void uartswInit(void)
{
	// initialize the buffers
	uartswInitBuffers();
	// initialize the ports
	sbi(UARTSW_TX_DDR, UARTSW_TX_PIN);
	#ifdef UARTSW_INVERT
	cbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
	#else
	sbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
	#endif
	#ifndef UARTSW_TX_ONLY
	cbi(UARTSW_RX_DDR, UARTSW_RX_PIN);
	#ifdef UARTSW_INVERT
	cbi(UARTSW_RX_PORT, UARTSW_RX_PIN);
	#else
	sbi(UARTSW_RX_PORT, UARTSW_RX_PIN);
	#endif
	#endif
	// initialize baud rate
	uartswSetBaudRate(9600);

	// setup the transmitter
	UartswTxBusy = FALSE;
	// disable OC1A interrupt
	cbi(TIMSK1, OCIE1A);
	// attach TxBit service routine to OC1A
	timerAttach(TIMER1OUTCOMPAREA_INT, uartswTxBitService);

	#ifndef UARTSW_TX_ONLY
	// setup the receiver
	UartswRxBusy = FALSE;
	// disable OC1B interrupt
	cbi(TIMSK1, OCIE1B);
	// attach RxBit service routine to OC1B
	timerAttach(TIMER1OUTCOMPAREB_INT, uartswRxBitService);
	// attach RxBit service routine to ICP
	timerAttach(TIMER1INPUTCAPTURE_INT, uartswRxBitService);
	#ifdef UARTSW_INVERT
	// trigger on rising edge
	sbi(TCCR1B, ICES1);
	#else
	// trigger on falling edge
	cbi(TCCR1B, ICES1);
	#endif
	// enable ICP interrupt
	sbi(TIMSK1, ICIE1);
	#endif

	// turn on interrupts
	sei();
}

// create and initialize the uart buffers
void uartswInitBuffers(void)
{
	#ifndef UARTSW_TX_ONLY
	// initialize the UART receive buffer
	bufferInit(&uartswRxBuffer, uartswRxData, UARTSW_RX_BUFFER_SIZE);
	#endif
}

// turns off software UART
void uartswOff(void)
{
	// disable interrupts
	cbi(TIMSK1, OCIE1A);
	#ifndef UARTSW_TX_ONLY
	cbi(TIMSK1, OCIE1B);
	cbi(TIMSK1, ICIE1);
	#endif
	// detach the service routines
	timerDetach(TIMER1OUTCOMPAREA_INT);
	#ifndef UARTSW_TX_ONLY
	timerDetach(TIMER1OUTCOMPAREB_INT);
	timerDetach(TIMER1INPUTCAPTURE_INT);
	#endif
}

void uartswSetBaudRate(uint32_t baudrate)
{
	// set timer prescaler
	timer1SetPrescaler(TIMER_CLK_DIV1);
	// calculate division factor for requested baud rate, and set it
	UartswBaudRateDiv = (uint16_t)((F_CPU+(baudrate/2L))/(baudrate*1L));
}

#ifndef UARTSW_TX_ONLY
// returns the receive buffer structure
cBuffer* uartswGetRxBuffer(void)
{
	// return rx buffer pointer
	return &uartswRxBuffer;
}
#endif

void uartswSendByte(uint8_t data)
{
	// wait until uart is ready
	while (UartswTxBusy);
	// set busy flag
	UartswTxBusy = TRUE;
	// save data
	UartswTxData = data;
	// set number of bits (+1 for stop bit)
	UartswTxBitNum = 9;

	// set the start bit
	#ifdef UARTSW_INVERT
	sbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
	#else
	cbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
	#endif

	// schedule the next bit
	outw(OCR1A, inw(TCNT1) + UartswBaudRateDiv);
	// enable OC1A interrupt
	sbi(TIMSK1, OCIE1A);
}

#ifndef UARTSW_TX_ONLY
// gets a single byte from the uart receive buffer (getchar-style)
int uartswGetByte(void)
{
	uint8_t c;
	if (uartswReceiveByte(&c))
		return c;
	else
		return -1;
}

// gets a byte (if avaiable) from the uart receive buffer
uint8_t uartswReceiveByte(uint8_t* rxData)
{
	// make sure we have a receive buffer
	if (uartswRxBuffer.size)
	{
		// make sure we have data
		if(uartswRxBuffer.datalength)
		{
			// get byte from beginning of buffer
			*rxData = bufferGetFromFront(&uartswRxBuffer);
			return TRUE;
		}
		else
		{
			// no data
			return FALSE;
		}
	}
	else
	{
		// no buffer
		return FALSE;
	}
}
#endif

void uartswTxBitService(void)
{
	if (UartswTxBitNum)
	{
		// there are bits still waiting to be transmitted
		if (UartswTxBitNum > 1)
		{
			// transmit data bits (inverted, LSB first)
			#ifdef UARTSW_INVERT
			if ( !(UartswTxData & 0x1) )
			#else
			if ( (UartswTxData & 0x01) )
			#endif
				sbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
			else
				cbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
			// shift bits down
			UartswTxData = UartswTxData>>1;
		}
		else
		{
			// transmit stop bit
			#ifdef UARTSW_INVERT
			cbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
			#else
			sbi(UARTSW_TX_PORT, UARTSW_TX_PIN);
			#endif
		}
		// schedule the next bit
		outw(OCR1A, inw(OCR1A) + UartswBaudRateDiv);
		// count down
		UartswTxBitNum--;
	}
	else
	{
		// transmission is done
		// clear busy flag
		UartswTxBusy = FALSE;
		// disable OC1A interrupt
		//cbi(TIMSK1, OCIE1A); <-- must comment out!!!
	}
}

#ifndef UARTSW_TX_ONLY
void uartswRxBitService(void)
{
	// this function runs on either:
	// - a rising edge interrupt
	// - OC1B
	if (!UartswRxBusy)
	{
		// this is a start bit
		// disable ICP interrupt
		cbi(TIMSK1, ICIE1);
		// schedule data bit sampling 1.5 bit periods from now
		outw(OCR1B, inw(TCNT1) + UartswBaudRateDiv + UartswBaudRateDiv/2);
		// clear OC1B interrupt flag
		sbi(TIFR1, OCF1B);
		// enable OC1B interrupt
		sbi(TIMSK1, OCIE1B);
		// set start bit flag
		UartswRxBusy = TRUE;
		// reset bit counter
		UartswRxBitNum = 0;
		// reset data
		UartswRxData = 0;
	}
	else
	{
		// start bit has already been received
		// we're in the data bits

		// shift data byte to make room for new bit
		UartswRxData = UartswRxData>>1;

		// sample the data line
		#ifdef UARTSW_INVERT
		if ( !(inb(UARTSW_RX_PORTIN) & (1<<UARTSW_RX_PIN)) )
		#else
		if ( (inb(UARTSW_RX_PORTIN) & (1<<UARTSW_RX_PIN)) )
		#endif
		{
			// serial line is marking
			// record '1' bit
			UartswRxData |= 0x80;
		}

		// increment bit count
		UartswRxBitNum++;
		// schedule next bit sample
		outw(OCR1B, inw(OCR1B) + UartswBaudRateDiv);

		// check if we have a full byte
		if (UartswRxBitNum >= 8)
		{
			// save data in receive buffer
			bufferAddToEnd(&uartswRxBuffer, UartswRxData);
			// disable OC1B interrupt
			cbi(TIMSK1, OCIE1B);
			// clear ICP interrupt flag
			sbi(TIFR1, ICF1);
			// enable ICP interrupt
			sbi(TIMSK1, ICIE1);
			// clear start bit flag
			UartswRxBusy = FALSE;
		}
	}
}
#endif
