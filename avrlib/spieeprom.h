// This library provides functions for reading and writing standard
// 25Cxxx/25LCxx SPI EEPROM memories. Memory sizes up to 64Kbytes are
// supported.

#ifndef SPIEEPROM_H
#define SPIEEPROM_H

#include "global.h"

// defines and constants
// commands
#define	SPIEEPROM_CMD_READ		0x03	// Read byte(s)
#define SPIEEPROM_CMD_WRITE		0x02	// Write byte(s)
#define SPIEEPROM_CMD_WREN		0x06	// Write Enable
#define SPIEEPROM_CMD_WRDI		0x04	// Write Disable
#define SPIEEPROM_CMD_RDSR		0x05	// Read Status Register
#define SPIEEPROM_CMD_WRSR		0x01	// Write Status Register

// status register bit defines
#define SPIEEPROM_STATUS_WIP	0		// Write in progress
#define SPIEEPROM_STATUS_WEL	1		// Write enable
#define SPIEEPROM_STATUS_BP0	2		// Block Protection 0
#define SPIEEPROM_STATUS_BP1	3		// Block Protection 1

#define SPIEEPROM_BYTES_PER_PAGE	64
#define SPIEEPROM_BYTES_MAX			0x7FFF

// functions

// Initialize SPI EEPROM interface
void spieepromInit(void);

// In the SPI EEPROM read a byte from memory location [memAddr]
uint8_t spieepromReadByte(uint16_t memAddr);

// In the SPI EEPROM write a byte [data] to the memory location [memAddr]
void spieepromWriteByte(uint16_t memAddr, uint8_t data);

void spieepromWriteEnable(void);
void spieepromWriteDisable(void);
uint8_t spieepromReadStatus(void);

// splits 16-bit address into 2 bytes, sends both
void spieepromSend16BitAddress(uint16_t memAddr);

// In the SPI EEPROM read a word from memory location [memAddr]
uint16_t spieeromReadWord(uint16_t memAddr);

// In the SPI EEPROM write a word [data] to the memory location [memAddr]
void spieepromWriteWord(uint16_t memAddr, uint16_t data);

// sets every byte in memory to zero
void spieepromClearAll(void);

#endif
