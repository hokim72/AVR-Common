#include <avr/io.h>

#include "spi.h"
#include "spieeprom.h"

// functions
void spieepromInit(void)
{
	// initialize SPI interface
	spiInit();
	sbi(SPIEEPROM_CS_DDR, SPIEEPROM_CS_PIN);
	sbi(SPIEEPROM_CS_PORT, SPIEEPROM_CS_PIN);

	// clock = f/128
	sbi(SPCR, SPR0);
	sbi(SPCR, SPR1);

	// enable SPI
	sbi(SPCR, SPE);
}

uint8_t spieepromReadByte(uint16_t memAddr)
{
	uint8_t data;
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_READ);
	// send address
	spieepromSend16BitAddress(memAddr);
	// read contents of memory address
	data  = spiTransferByte(0xFF);
	unselectEEPROM();
	// return data
	return data;
}

void spieepromWriteByte(uint16_t memAddr, uint8_t data)
{

	spieepromWriteEnable();
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRITE);
	// send address
	spieepromSend16BitAddress(memAddr);
	// send data to be written
	spiTransferByte(data);
	unselectEEPROM();
	// wait for write to complete
	while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
}

void spieepromWriteEnable(void)
{
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WREN);
	unselectEEPROM();
}

void spieepromWriteDisable(void)
{
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRDI);
	unselectEEPROM();
}

uint8_t spieepromReadStatus(void)
{
	uint8_t status;
	selectEEPROM();
	spiTransferByte(SPIEEPROM_CMD_RDSR);
	// get status register value
	status = spiTransferByte(0xFF);
	unselectEEPROM();
	return status;
}

void spieepromSend16BitAddress(uint16_t memAddr)
{
	spiTransferByte((uint8_t)memAddr>>8);
	spiTransferByte((uint8_t)memAddr);
}

uint16_t spieepromReadWord(uint16_t memAddr)
{
	uint16_t data;
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_READ);
	// send address
	spieepromSend16BitAddress(memAddr);
	// read contents of memory address
	data = (spiTransferByte(0xFF) << 8);
	data |= spiTransferByte(0xFF);
	unselectEEPROM();
	return data;
}

void spieepromWriteWord(uint16_t memAddr, uint16_t data)
{
	spieepromWriteEnable();
	selectEEPROM();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRITE);
	// send address
	spieepromSend16BitAddress(memAddr);
	// send data to be written
	spiTransferByte((uint8_t)(data>>8));
	spiTransferByte((uint8_t)data);
	unselectEEPROM();
	// wait for write to complete
	while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
}

void spieepromClearAll(void)
{
	uint8_t i;
	uint16_t pageAddress = 0;
	while (pageAddress < SPIEEPROM_BYTES_MAX)  {
		spieepromWriteEnable();
		selectEEPROM();
		spiTransferByte(SPIEEPROM_CMD_WRITE);
		spieepromSend16BitAddress(pageAddress);
		for (i=0; i<SPIEEPROM_BYTES_PER_PAGE; i++) {
			spiTransferByte(0);
		}
		unselectEEPROM();
		pageAddress += SPIEEPROM_BYTES_PER_PAGE;
		while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
	}
}
