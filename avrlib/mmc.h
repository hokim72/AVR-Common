// This library offers some simple function which can be used to read and
// write data on a MultiMedia or SecureDigital (SD) Flash Card.
// Although MMC and SD Cards are designed to operate with their own
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

#ifndef MMC_H
#define MMC_H

#include "global.h"
#include <avr/io.h>

#include "mmcconf.h"

// Definitions for MMC/SDC command
#define	GO_IDLE_STATE			0			// CMD0
#define SEND_OP_COND_MMC		1			// CMD1
#define SEND_OP_COND_SDC		(0x80+41)	// ACMD41
#define SEND_IF_COND			8			// CMD8
#define SEND_CSD				9			// CMD9
#define SEND_CID				10			// CMD10
#define STOP_TRANSMISSION		12			// CMD12
#define SEND_STATUS_SDC			(0x80+13)	// ACDM13
#define SET_BLOCK_LEN			16			// CMD16
#define READ_SINGLE_BLOCK		17			// CMD17
#define READ_MULTIPLE_BLOCK		18			// CMD18
#define SET_BLOCK_COUNT_MMC		23			// CMD23
#define SET_WR_BLK_ERASE_COUNT_SDC (0x80+23)// ACMD23
#define WRITE_BLOCK				24			// CMD24
#define WRITE_MULTIPLE_BLOCK	25			// CMD25
#define ERASE_ER_BLK_START		32			// CMD32
#define ERASE_ER_BLK_END		33			// CMD33
#define ERASE					38			// CMD38
#define	APP_CMD					55			// CMD55
#define READ_OCR				58			// CMD58


extern uint8_t cardType;
// functions

int receiveDataBlock(uint8_t* buf, uint32_t btr);
int sendDataBlock(const uint8_t* buf, uint8_t token);

void mmcDeselect(void);
int mmcSelect(void);

uint8_t mmcSendCommand(uint8_t cmd, uint32_t arg);

// Initialize AVR<->MMC hardware interface
// Prepares hardware for MMC access.
void mmcInit(void);

// Initialize the card and prepare it for use.
// Returns zero if successful
uint8_t mmcReset(void);

// Read Sector(s)
uint8_t mmcRead(uint8_t* buf, uint32_t sector, uint32_t count);

// Write Sector(s)
uint8_t mmcWrite(const uint8_t* buf, uint32_t sector, uint32_t count);


#endif
