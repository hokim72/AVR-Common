// This library offers some simple function which can be used to read and
// write data on a MultiMedia or SecureDigital (SD) Flash Card.
// Although MM and SD Cards are designed to operate with their own
// special bus wiring and protocols, both types of cards also provid a
// simple SPI-like interface mode which is exceptionally useful when
// attempting to use the cards in embedded systems.
//
// Wring
//	To work with this library, the card must be wired to the SPI port of
//	the Atmel microcontroller as described below. Typical cards can operate
//	at up to 25MHz maximum SPI clock rate (thus faster than most AVR's
//	maximum SPI clock rate).
//       _________________
//      /  1 2 3 4 5 6 78 |    <- view of MMC/SD card looking at contacts
//     / 9                |    Pins 8 and 9 are present only on SD cards
//     |    MMC/SD Card   |
//     |                  |
//     
//     1 - CS   (chip select)          - wire to any available I/O pin(*)
//     2 - DIN  (data in, card<-host)  - wire to SPI MOSI pin
//     3 - VSS  (ground)               - wire to ground
//     4 - VDD  (power, 3.3V only?)    - wire to power (MIGHT BE 3.3V ONLY!)
//     5 - SCLK (data clock)           - wire to SPI SCK pin
//     6 - VSS  (ground)               - wire to ground
//     7 - DOUT (data out, card->host) - wire to SPI MISO pin
//
//		(*) you must define this chip select I/O pin in sdconf.h

#ifndef SD_H
#define SD_H

#include "global.h"
#include <avr/io.h>

#include "sdconf.h"

// constants/macros/typdefs
// www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf
// SD commands (taken from sandisk SD reference)
#define	GO_IDLE_STATE			0x00	// init card in spi mode if CS low
#define SEND_IF_COND			0x08	// verify SD Memory Card interface operating condition
//#define SEND_CSD				0x09	// read the Card Specific Data (CSD register)
//#define SEND_CID				0x0A	// read the card identification information (CID register)
#define STOP_TRANSMISSION		0x0C	// stop transmission
//#define SEND_STATUS				0x0D	// read the card status register
//#define SET_BLOCK_LEN			0x10	// set block size
#define READ_SINGLE_BLOCK		0x11	// read a single data block from the card
#define READ_MULTIPLE_BLOCKS	0x12	// read a multiple data blocks from the card
#define SET_WR_BLK_ERASE_COUNT	0x17	// Set the number of write blocks to be pre-erased before writing
#define WRITE_SINGLE_BLOCK		0x18	// write a single data block to the card
#define WRITE_MULTIPLE_BLOCKS	0x19	// write blocks of data until a STOP_TRANSMISSION
#define ERASE_BLOCK_START_ADDR	0x20	// sets the address of the first block to be erased
#define ERASE_BLOCK_END_ADDR	0x21	// sets the address of the last block of the continuous range to be erased
#define ERASE_SELECTED_BLOCKS	0x26	// erase all previously selected blocks
#define SD_SEND_OP_COMD			0x29	// Sends host capacity support informationi; ACMD
#define	APP_CMD					0x37	// escape for application specific command
#define READ_OCR				0x3A	// read the OCR register of a card
#define	CRC_ON_OFF				0x3B	// disable CRC

// status
#define	R1_READY_STATE			0x00	// status for card in the ready state
#define	R1_IDLE_STATE			0x01	// status for card in the idle state
#define	R1_ILLEGAL_COMMAND		0x04	// status bit for illegal command
#define	DATA_START_BLOCK		0xFE	// start data token for read or write single block
#define	STOP_TRAN_TOKEN			0xFD	// stop token for write multiple blocks
#define WRITE_MULTIPLE_TOKEN	0xFC	// start data token for write multiple blocks
#define	DATA_RES_MASK			0x1F	// mask for data response tokens after a write block operation
#define DATA_RES_ACCEPTED		0x05	// write data accepted token

// card types
#define SD_CARD_TYPE_SD1		1 		// Standard capacity V1 SD card
#define SD_CARD_TYPE_SD2		2		// Standard capacity V2 SD card
#define SD_CARD_TYPE_SDHC		3		// High Capacity SD card


// functions

// Initialize AVR<->SD hardware interface
// Prepares hardware for SD access.
void sdInit(void);

// Initialize the card and prepare it for use.
// Returns zero if successful
uint8_t sdReset(void);

// Read 512-byte sector from card to buffer
// Returns zero if successful.
uint8_t sdReadSingleBlock(uint32_t sector, uint8_t* buffer);

// Write 512-byte sector from buffer to card
// Return zero if successful
uint8_t sdWriteSingleBlock(uint32_t sector, uint8_t* buffer);

uint8_t sdReadMultipleBlock(uint32_t startSector, uint32_t totalSectors, uint8_t* buffer);
uint8_t sdWriteMultipleBlock(uint32_t startSector, uint32_t totalSectors, uint8_t* buffer);
uint8_t sdErase(uint32_t startSector, uint32_t totalSectors);

// Send card an SD command.
// Returns response result code.
uint8_t sdSendCommand(uint8_t cmd, uint32_t arg);

#endif
