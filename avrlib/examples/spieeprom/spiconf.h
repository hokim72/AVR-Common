#ifndef SPICONF_H
#define SPICONF_H
// If the physical SS pin is configured as input, it must be held high to ensure Master SPI operation
#define SPI_SS_DDR				DDRB
#define SPI_SS_PORT				PORTB
#define SPI_SS_PIN				PB2

//#define SPI_CS_PORT			PORTB
//#define SPI_CS_DDR			DDRB
//#define	SPI_CS_PIN			PB2

#define SPI_MOSI_DDR			DDRB
#define SPI_MOSI_PIN			PB3

#define SPI_MISO_PORT			PORTB
#define SPI_MISO_DDR			DDRB
#define SPI_MISO_PIN			PB4

#define SPI_SCK_DDR				DDRB
#define SPI_SCK_PIN				PB5

//#define selectSlave()  	cbi(SPI_CS_PORT, SPI_CS_PIN)
//#define unselectSlave() sbi(SPI_CS_PORT, SPI_CS_PIN)

#endif
