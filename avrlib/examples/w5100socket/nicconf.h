//Project specific configurations
#ifndef __GLOBAL_CONF_H__
#define __GLOBAL_CONF_H__

//Define frequency
//#define F_CPU 16000000UL
//Mac adress definition for enc28j60
#define ETHADDR0		0x00
#define ETHADDR1		0x01
#define ETHADDR2		0x02
#define ETHADDR3		0x03
#define ETHADDR4		0x04
#define ETHADDR5		0x05

// W5100 SPI port
#define W5100_SPI_PORT			PORTB
#define W5100_SPI_DDR			DDRB

// pre ATMega328p 
#ifdef PB5
#define W5100_SPI_SCK			PB5
#define W5100_SPI_MOSI			PB3
#define W5100_SPI_MISO			PB4
#define W5100_SPI_SS			PB2
#define W5100_CONTROL_CS		PB2
#else
#define W5100_SPI_SCK    	    PORTB5
#define W5100_SPI_MOSI 	        PORTB3
#define W5100_SPI_MISO          PORTB4
#define W5100_SPI_SS 	        PORTB2
#define W5100_CONTROL_CS   	 	PORTB2
#endif

// W5100 control port
#define W5100_CONTROL_PORT		PORTB
#define W5100_CONTROL_DDR		DDRB

#endif /*__GLOBAL_CONF_H__*/
