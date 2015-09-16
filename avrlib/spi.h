// Provides basic byte and word transmitting and receiving via the AVR SPI
// interface. Due to the nature of SPI, every SPI communication operation is
// both a transmit and simultaneous receive
// Currently, only MASTER mode is supported.

#ifndef SPI_H
#define SPI_H

#include "global.h"
#include <avr/io.h>

// includ configuration
#include "spiconf.h"

// function prototypes

// SPI interface initializer
void spiInit(void);

// spiSendByte(uint8_t data) waits until the SPI interface is ready and the
// sends a single byte over the SPI port. This command does not receive 
// anything.
void spiSendByte(uint8_t data);

// spiTransferByte(uint8_t data) waits until the SPI interface is ready
// and then sends a single byte over the SPI port. The function also
// returns the byte that was received during transmission.
uint8_t spiTransferByte(uint8_t data);

// spiTransferWord(uint16_t data) works just like spiTransferByte but
// operates on a whole word (16-bits of data).
uint16_t spinTransferWord(uint16_t data);

// Send a data block fast
void spiSendMulti(uint8_t* p, uint32_t cnt);

// Receive a data block fast
void spiReceiveMulti(uint8_t* p, uint32_t cnt);

#endif
