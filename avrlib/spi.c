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
	//sbi(SPI_SS_PORT, SPI_SS_PIN); // physical ss should be held high when input
	sbi(SPI_SS_DDR, SPI_SS_PIN);    // or it should be output  
	sbi(SPI_SS_PORT, SPI_SS_PIN);
	//sbi(SPI_CS_DDR, SPI_CS_PIN);
	//sbi(SPI_CS_PORT, SPI_CS_PIN);
	sbi(SPI_MOSI_DDR, SPI_MOSI_PIN);
	sbi(SPI_MISO_PORT, SPI_MISO_PIN);
	sbi(SPI_SCK_DDR, SPI_SCK_PIN);

	// setup SPI interface :
	// master mode
	sbi(SPCR, MSTR);
	// clock = f/128
	//sbi(SPCR, SPR0);
	//sbi(SPCR, SPR1);

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
		// while (!(inb(SPSR) & (1<<SPIF)));
		loop_until_bit_is_set(SPSR, SPIF);
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

// p: Data block to sent; cnt: Size of data block (must be multiple of 2)
void spiSendMulti(uint8_t* p, uint32_t cnt)
{
	do {
		SPDR = *p++; loop_until_bit_is_set(SPSR, SPIF);
		SPDR = *p++; loop_until_bit_is_set(SPSR, SPIF);
	} while (cnt -= 2);
}

// p: Data buffer; Size of data block (must be multiple of 2)
void spiReceiveMulti(uint8_t* p, uint32_t cnt)
{
	do {
		SPDR = 0xFF; loop_until_bit_is_set(SPSR, SPIF); *p++ = SPDR;
		SPDR = 0xFF; loop_until_bit_is_set(SPSR, SPIF); *p++ = SPDR;
	} while (cnt -= 2);
}
