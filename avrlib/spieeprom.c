#include <avr/io.h>

#include "spi.h"
#include "spieeprom.h"

// functions
void spieepromInit(void)
{
	// initialize SPI interface
	spiInit();
}

uint8_t spieepromReadByte(uint16_t memAddr)
{
	uint8_t data;
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_READ);
	// send address
	spieepromSend16BitAddress(memAddr);
	// read contents of memory address
	data  = spiTransferByte(0xFF);
	unselectSlave();
	// return data
	return data;
}

void spieepromWriteByte(uint16_t memAddr, uint8_t data)
{

	spieepromWriteEnable();
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRITE);
	// send address
	spieepromSend16BitAddress(memAddr);
	// send data to be written
	spiTransferByte(data);
	unselectSlave();
	// wait for write to complete
	while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
}

void spieepromWriteEnable(void)
{
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WREN);
	unselectSlave();
}

void spieepromWriteDisable(void)
{
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRDI);
	unselectSlave();
}

uint8_t spieepromReadStatus(void)
{
	uint8_t status;
	selectSlave();
	spiTransferByte(SPIEEPROM_CMD_RDSR);
	// get status register value
	status = spiTransferByte(0xFF);
	unselectSlave();
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
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_READ);
	// send address
	spieepromSend16BitAddress(memAddr);
	// read contents of memory address
	data = (spiTransferByte(0xFF) << 8);
	data |= spiTransferByte(0xFF);
	unselectSlave();
	return data;
}

void spieepromWriteWord(uint16_t memAddr, uint16_t data)
{
	spieepromWriteEnable();
	selectSlave();
	// send command
	spiTransferByte(SPIEEPROM_CMD_WRITE);
	// send address
	spieepromSend16BitAddress(memAddr);
	// send data to be written
	spiTransferByte((uint8_t)(data>>8));
	spiTransferByte((uint8_t)data);
	unselectSlave();
	// wait for write to complete
	while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
}

void spieepromClearAll(void)
{
	uint8_t i;
	uint16_t pageAddress = 0;
	while (pageAddress < SPIEEPROM_BYTES_MAX)  {
		spieepromWriteEnable();
		selectSlave();
		spiTransferByte(SPIEEPROM_CMD_WRITE);
		spieepromSend16BitAddress(pageAddress);
		for (i=0; i<SPIEEPROM_BYTES_PER_PAGE; i++) {
			spiTransferByte(0);
		}
		unselectSlave();
		pageAddress += SPIEEPROM_BYTES_PER_PAGE;
		while (spieepromReadStatus() & BV(SPIEEPROM_STATUS_WIP));
	}
}
