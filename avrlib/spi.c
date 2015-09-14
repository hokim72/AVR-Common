#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"

// Define the SPI_USEINT key if you want SPI bus operation to be
// interrupt-driven. The primary reason for not using SPI in interrupt-driven
// mode is if the SPI send/transfer commands will be used from within some
// other interrupt service routine or if interrupts might be globally turned
// off due to of other aspects of your program
//
// Comment-out or uncommont this line as necessary
//#define SPI_USEINT

#ifdef SPI_USEINT
// global variables
volatile uint8_t spiTransferComplete;

// SPI interrupt service handler
ISR(SPI_STC_vect)
{
	spiTransferComplete = TRUE;
}
#endif

// access routines
void spiInit()
{
// setup SPI I/O pins
	sbi(SPI_PSS_PORT, SPI_PSS);
	sbi(SPI_SS_DDR, SPI_SS);
	sbi(SPI_SS_PORT, SPI_SS);
	sbi(SPI_MOSI_DDR, SPI_MOSI);
	sbi(SPI_MISO_PORT, SPI_MISO);
	sbi(SPI_SCK_DDR, SPI_SCK);

	// setup SPI interface :
	// master mode
	sbi(SPCR, MSTR);
	// clock = f/64
	cbi(SPCR, SPR0);
	sbi(SPCR, SPR1);
	// select clock phase positive-going in middle of data
	cbi(SPCR, CPOL);
	// Data order MSB first
	cbi(SPCR, DORD);
	// enable SPI
	sbi(SPCR, SPE);


	#ifdef SPI_USEINT
	// clear status
	inb(SPSR);
	spiTransferComplete = TRUE;
	// enable SPI interrupt
	sbi(SPCR, SPIE);
	#endif
}

void spiSendByte(uint8_t data)
{
	// send a byte over SPI and ignore reply
	// send the given data
	#ifdef SPI_USEINT
	spiTransferComplete = FALSE;
	#endif
	outb(SPDR, data);
	#ifdef SPI_USEINT
		// wait for transfer to complete
		while (!spiTransferComplete);
	#else
		// wait for transfer to complete
		while (!(inb(SPSR) & (1<<SPIF)));
	#endif
}

uint8_t spiTransferByte(uint8_t data)
{
	spiSendByte(data);
	// return the received data
	return inb(SPDR);
}

uint16_t spiTransferWord(uint16_t data)
{
	uint16_t rxData = 0;
	// send MS byte of give data
	rxData = (spiTransferByte((data>>8) & 0x00FF)) <<8;
	// send LS byte of give data
	rxData |= (spiTransferByte(data & 0x00FF));

	// return the received data
	return rxData;
}
