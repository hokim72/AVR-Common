#include <avr/io.h>

// If the physical SS pin is configured as input, it must be held high to ensure Master SPI operation
#define SPI_PSS_PORT		PORTB
#define SPI_PSS				PB2

#define SPI_SS_PORT			PORTD
#define SPI_SS_DDR			DDRD
#define	SPI_SS				PD4

#define SPI_MOSI_DDR		DDRB
#define SPI_MOSI			PB3

#define SPI_MISO_PORT		PORTB
#define SPI_MISO_DDR		DDRB
#define SPI_MISO			PB4

#define SPI_SCK_DDR			DDRB
#define SPI_SCK				PB5

#define selectSlave()  	cbi(SPI_SS_PORT, SPI_SS)
#define unselectSlave() sbi(SPI_SS_PORT, SPI_SS)

