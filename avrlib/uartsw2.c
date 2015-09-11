#include <avr/io.h>
#include <avr/interrupt.h>

#include "global.h"
#include "timer.h"
#include "uartsw2.h"

// uartsw transmit status and data variables
static volatile uint8_t UartswTxBusy;
static volatile uint8_t UartswTxData;
static volatile uint8_t UartswTxBitNum;

// baud rate common to transmit and receive
static volatile uint8_t UartswBaudRateDiv;

// uartsw receive status and data variables
static volatile uint8_t UartswRxBusy;
static volatile uint8_t UartswRxData;
static volatile uint8_t UartswRxBitNum;
// receive buffer
static cBuffer uartswRxBuffer;					// uartsw receive buffer
// automatically allocate space in ram for each buffer
static unsigned char uartswRxData[UARTSW_RX_BUFFER_SIZE];

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
	cbi(UARTSW_RX_DDR, UARTSW_RX_PIN);
	#ifdef UARTSW_INVERT
	cbi(UARTSW_RX_PORT, UARTSW_RX_PIN);
	#else
	sbi(UARTSW_RX_PORT, UARTSW_RX_PIN);
	#endif
	// initialize baud rate
	uartswSetBaudRate(9600);

	// setup the transmitter
	UartswTxBusy = FALSE;
	// disable OC2A interrupt
	cbi(TIMSK2, OCIE2A);
	// attach TxBit service routine to OC2A
	timerAttach(TIMER2OUTCOMPAREA_INT, uartswTxBitService);

	// setup the receiver
	UartswRxBusy = FALSE;
	// disable 0C2B interrupt
	cbi(TIMSK2, OCIE2B);
	// attach RxBit service routine to OC2B
	timerAttach(TIMER2OUTCOMPAREB_INT, uartswRxBitService);
	// INT1 trigger on rising/falling edge
	#ifdef UARTSW_INVERT
	sbi(EICRA, ISC11);
	sbi(EICRA, ISC10);
	#else
	sbi(EICRA, ISC11);
	cbi(EICRA, ISC10);
	#endif
	// enable INT1 interrupt
	sbi(EIMSK, INT1);

	// turn on interrupts
	sei();
}

// create and initialize the uart buffers
void uartswInitBuffers(void)
{
	// initialize the UART receive buffer
	bufferInit(&uartswRxBuffer, uartswRxData, UARTSW_RX_BUFFER_SIZE);
}

// turns off software UART
void uartswOff(void)
{
	// disable interrupts
	cbi(TIMSK2, OCIE2A);
	cbi(TIMSK2, OCIE2B);
	cbi(EIMSK, INT1);
	// detach the service routines
	timerDetach(TIMER2OUTCOMPAREA_INT);
	timerDetach(TIMER2OUTCOMPAREB_INT);
}

void uartswSetBaudRate(uint32_t baudrate)
{
	uint16_t div;

	// set timer prescaler
	if ( baudrate > (F_CPU/64L*256L) )
	{
		// if the requested baud rate is high,
		// set timer prescalers to div-by-64
		timer2SetPrescaler(TIMER_CLK_DIV64);
		div = 64;
	}
	else
	{
		// if the requested baud rate is low,
		// set timer prescalers to div-by-256
		timer2SetPrescaler(TIMER_CLK_DIV256);
		div = 256;
	}

	// calculate division factor for requested baud rate, and set it
	//UartswBaudRateDiv = (uint8_t)(((F_CPU/64L)+(baudrate/2L))/(baudrate*1L));
	//UartswBaudRateDiv = (uint8_t)(((F_CPU/256L)+(baudrate/2L))/(baudrate*1L));
	UartswBaudRateDiv = (uint8_t)(((F_CPU/div)+(baudrate/2L))/(baudrate*1L));
}

// returns the receive buffer structure
cBuffer* uartswGetRxBuffer(void)
{
	// return rx buffer pointer
	return &uartswRxBuffer;
}

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
	outb(OCR2A, inb(TCNT2) + UartswBaudRateDiv);
	// enable OC2A interrupt
	sbi(TIMSK2, OCIE2A);
}

// gets a single byte from the uart receive buffer (getchar-style)
int uartswGetByte(void)
{
	uint8_t c;
	if (uartswReceiveByte(&c))
		return c;
	else
		return -1;
}

// gets a byte (if available) from the uart receive buffer
uint8_t uartswReceiveByte(uint8_t* rxData)
{
	// make sure we have a receive buffer
	if (uartswRxBuffer.size)
	{
		// make sure we have data
		if (uartswRxBuffer.datalength)
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

void uartswTxBitService(void)
{
	if (UartswTxBitNum)
	{
		// there are bits still waiting to be transmitted
		if (UartswTxBitNum > 1)
		{
			// transmit data bits (inverted, LSB first)
			#ifdef UARTSW_INVERT
			if ( !(UartswTxData & 0x01) )
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
		outb(OCR2A, inb(OCR2A) + UartswBaudRateDiv);
		// count down
		UartswTxBitNum--;
	}
	else
	{
		// transmission is done
		// clear busy flag
		UartswTxBusy = FALSE;
		// disable OC2A interrupt
		//cbi(TIMSK2, OCIE2A);  <-- must comment out!!! 
	}
}

void uartswRxBitService(void)
{
	// this function runs on either:
	// - a rising edge interrupt
	// - Timer 0 output compare
	if (!UartswRxBusy)
	{
		// UART was not previously busy,
		// this must be is a start bit

		// disable INT1 interrupt
		cbi(EIMSK, INT1);
		// schedule data bit sampling 1.5 bit periods from now
		outb(OCR2B, inb(TCNT2) + UartswBaudRateDiv + UartswBaudRateDiv/2);
		// clear OC2B interrupt flag
		sbi(TIFR2, OCF2B);
		// enable OC2B interrupt
		sbi(TIMSK2, OCIE2B);
		// set busy flag
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

		// increment bit counter
		UartswRxBitNum++;
		// schedule next bit sample
		outb(OCR2B, inb(OCR2B) + UartswBaudRateDiv);

		// check if we have a full byte
		if (UartswRxBitNum >= 8)
		{
			// save data in receive buffer
			bufferAddToEnd(&uartswRxBuffer, UartswRxData);
			// disable 0C2B interrupt
			cbi(TIMSK2, OCIE2B);
			// clear INT1 interrupt flag
			sbi(EIFR, INTF1);
			// enable INT1 interrupt
			sbi(EIMSK, INT1);
			// clear busy flag
			UartswRxBusy = FALSE;
		}
	}
}

ISR(INT1_vect)
{
	// run RxBit service routine
	uartswRxBitService();
}
