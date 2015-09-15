#include <avr/io.h>

#include "spi.h"
#include "sd.h"
#ifdef SD_DEBUG
#include "rprintf.h"
#endif

#define NTRIES	100

static volatile uint8_t sdCardType;

// functions
void sdInit(void)
{
	// initialize SPI interface
	spiInit();
	sbi(SD_CS_DDR, SD_CS_PIN);
	sbi(SD_CS_PORT, SD_CS_PIN);

	// enable SPI
	sbi(SPCR, SPE);
}

uint8_t sdReset(void)
{
	uint8_t i;
	uint8_t retry;
	uint8_t res;
	uint32_t arg;

	// card needs 74 cycles minimum to start up
	for (i=0; i<10; i++) spiTransferByte(0xFF);

	selectSD();

	// command to go idle in SPI mode
	retry = 0;
	do {
		res = sdSendCommand(GO_IDLE_STATE, 0);
		#ifdef SD_DEBUG
		rprintf("GO_IDLE_STAT -> 0x%x\n", res);
		#endif
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_IDLE_STATE);

	// check SD version
	if ((res=sdSendCommand(SEND_IF_COND, 0x1AA)) & R1_ILLEGAL_COMMAND) {
		sdCardType = SD_CARD_TYPE_SD1;
	} else {
		// only need last byte of r7 response
		for (i=0; i<4; i++) res = spiTransferByte(0xFF);
		if (res != 0xAA) {
			goto fail;
		}
		sdCardType = SD_CARD_TYPE_SD2;
	}
	#ifdef SD_DEBUG
	rprintf("SEND_IF_COND -> 0x%x\n", res);
	#endif
	// initialize card and send host supports SDHC if SD2
	arg = sdCardType == SD_CARD_TYPE_SD2 ? 0x40000000 : 0;
	retry = 0;
	do {
		sdSendCommand(APP_CMD, 0);
		res = sdSendCommand(SD_SEND_OP_COMD, arg);
		#ifdef SD_DEBUG
		rprintf("SD_SEND_OP_COMD -> 0x%x\n", res);
		#endif
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_READY_STATE);

	// if SD2 read OCR register to check for SDHC card
	if (sdCardType == SD_CARD_TYPE_SD2) {
		if (sdSendCommand(READ_OCR, 0)) goto fail;
		if ((spiTransferByte(0xFF) & 0xC0) == 0xC0) sdCardType = SD_CARD_TYPE_SDHC;
		// discard rest of ocr - contains allowed voltage range
		for (i=0; i<3; i++) spiTransferByte(0xFF);
	}

	// sdSendCommand(CRC_ON_OFF, 0); // 1: ON, 0: OFF; disable CRC; default - CRC disabled in SPI mode
	// sdSendCommand(SET_BLOCK_LEN, 512); // set block size to 512; default size is 512

	unselectSD();
	#ifdef SD_DEBUG
	rprintf("card type = %d\n", sdCardType);
	rprintf("Initialization Success!!!\n");
	#endif

	///* switch to highest SPI frequency possible */
	//SPCR &= ~((1 << SPR1) | (1 << SPR0)); /* Clock Frequency: f_OSC / 4 */
	//SPSR |= (1 << SPI2X); /* Doubled Clock Frequency: f_OSC / 2 */
	return 0;

	fail:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Initialization Fail!!!\n");
		#endif
		return -1;
}

uint8_t sdReadSingleBlock(uint32_t sector, uint8_t* buffer)
{
	uint16_t i;
	uint8_t retry;
	uint8_t res;

	selectSD();

	res = sdSendCommand(READ_SINGLE_BLOCK, sector); // read a Block command

	#ifdef SD_DEBUG
	rprintf("READ_SINGLE_BLOCK -> 0x%x\n", res);
	#endif
	if (res != R1_READY_STATE) {
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}

	retry = 0;
	do {
		res = spiTransferByte(0xFF);	
		#ifdef SD_DEBUG
		rprintf("read data header -> 0x%x\n", res);
		#endif
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != DATA_START_BLOCK); // wait for start block token 0xfe
	for (i=0; i<512; i++) // read 512 bytes
	{
		buffer[i] = spiTransferByte(0xFF);
	}
	spiTransferByte(0xFF); // receive incoming CRC (16-bit), CRC is ignore here
	spiTransferByte(0xFF);

	spiTransferByte(0xFF); // extra 8 clock pusles
	unselectSD();
	#ifdef SD_DEBUG
	rprintf("Read Single Block Success!!!\n");
	#endif
	return 0;

	fail:
		res = -1;
	fail_res:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Read Single Fail!!!\n");
		#endif
		return res;
}

uint8_t sdWriteSingleBlock(uint32_t sector, uint8_t* buffer)
{
	uint16_t i;
	uint8_t retry;
	uint8_t res;

	selectSD();

	res = sdSendCommand(WRITE_SINGLE_BLOCK, sector); // write a Block command
	if (res != R1_READY_STATE) {
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	#ifdef SD_DEBUG
	rprintf("WRITE_SINGLE_BLOCK -> 0x%x\n", res);
	#endif

	spiTransferByte(DATA_START_BLOCK); // Send start block token 0xfe
	for (i=0; i<512; i++) // send 512 bytes data
	{
		spiTransferByte(buffer[i]);
	}

	spiTransferByte(0xFF); // transmit dummy CRC (16-bit), CRC is ignored here
	spiTransferByte(0xFF);

	res = spiTransferByte(0xFF);

	#ifdef SD_DEBUG
	rprintf("write accept? -> 0x%x\n", res);
	#endif

	//response= 0xXXX0AAA1 ; AAA='010' - data accepted
	//AAA='101'-data rejected due to CRC error
	//AAA='110'-data rejected due to write error
	if ((res &  DATA_RES_MASK) != DATA_RES_ACCEPTED) {
		goto fail_res; 
	}

	// wait for SD card to complete writing and get idle
	retry = 0;
	do {
		res = spiTransferByte(0xFF);	
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_READY_STATE);

	unselectSD();
	spiTransferByte(0xFF);	// just spend 8 clock cycle delay before reasserting the CS line
	selectSD();	// re-asserting the CS line to verify if card is still busy
	
	// wait for SD card to complete writing and get idle
	retry = 0;
	do {
		res = spiTransferByte(0xFF);	
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_READY_STATE);
	unselectSD();


	#ifdef SD_DEBUG
	rprintf("Write Single Block Success!!!\n");
	#endif
	return 0;

	fail:
		res = -1;
	fail_res:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Write Single Block Fail!!!\n");
		#endif
		return res;
}

uint8_t sdReadMultipleBlock(uint32_t startSector, uint32_t totalSectors, uint8_t* buffer) 
{
	uint16_t i;
	uint16_t ibyte;
	uint8_t retry;
	uint8_t res;

	selectSD();

	res = sdSendCommand(READ_MULTIPLE_BLOCKS, startSector); // write a Block command
	#ifdef SD_DEBUG
	rprintf("READ_MULTIPLE_BLOCKS -> 0x%x\n", res);
	#endif
	if (res != R1_READY_STATE) {
		goto fail_res;	// check for SD status: 0x00 - OK (No flags set)
	}

	ibyte = 0;
	while (totalSectors)
	{
		retry = 0;
		do {
			res = spiTransferByte(0xFF);
			#ifdef SD_DEBUG
			rprintf("read data header -> 0x%x\n", res);
			#endif
			retry++;
			if (retry > NTRIES) goto fail;
		} while(res != DATA_START_BLOCK); // wait for start block token 0xfe
		for (i=0; i<512; i++) // read 512 bytes
		{
			buffer[ibyte+i] = spiTransferByte(0xFF);
		}
		spiTransferByte(0xFF); // receive incoming CRC (16-bit), CRC is ignore here
		spiTransferByte(0xFF);

		spiTransferByte(0xFF); // extra 8 clock pulse

		totalSectors--;
		ibyte += 512;
	}

	sdSendCommand(STOP_TRANSMISSION, 0); // command to stop transmission
	unselectSD();
	spiTransferByte(0xFF); // extra 8 clock pulses

	#ifdef SD_DEBUG
	rprintf("Read Multiple Block Success!!!\n");
	#endif
	return 0;

	fail:
		res = -1;
	fail_res:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Read Multiple Block Fail!!!\n");
		#endif
		return res;
}
uint8_t sdWriteMultipleBlock(uint32_t startSector, uint32_t totalSectors, uint8_t* buffer) 
{
	uint16_t i;
	uint16_t ibyte;
	uint8_t retry;
	uint8_t res;

	selectSD();

	// send pre-erase count
	sdSendCommand(APP_CMD, 0);
	res = sdSendCommand(SET_WR_BLK_ERASE_COUNT, totalSectors);
	if (res != R1_READY_STATE) {
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	
	// use address if not SDHC card
	res = sdSendCommand(WRITE_MULTIPLE_BLOCKS, sdCardType !=  SD_CARD_TYPE_SDHC ? startSector<<9 : startSector); // write a Block command
	if (res != R1_READY_STATE) {
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	#ifdef SD_DEBUG
	rprintf("WRITE_MULTIPLE_BLOCKS -> 0x%x\n", res);
	#endif

	ibyte = 0;
	while (totalSectors)
	{

		spiTransferByte(WRITE_MULTIPLE_TOKEN); // Send start block token 0xfc
		for (i=0; i<512; i++) // send 512 bytes data
		{
			spiTransferByte(buffer[ibyte+i]);
		}

		spiTransferByte(0xFF); // trasmit dummy CRC (16-bit), CRC is ignored here
		spiTransferByte(0xFF);

		res = spiTransferByte(0xFF);

		#ifdef SD_DEBUG
		rprintf("write accept? -> 0x%x\n", res);
		#endif

		//response= 0xXXX0AAA1 ; AAA='010' - data accepted
		//AAA='101'-data rejected due to CRC error
		//AAA='110'-data rejected due to write error
		if ((res & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
			goto fail_res;
		}

		// wait for SD card to complete writing and get idle
		retry = 0;
		do {
			res = spiTransferByte(0xFF);
			retry++;
			if (retry > NTRIES) goto fail;
		} while(res != R1_READY_STATE);

		spiTransferByte(0xFF); // extra 8 bits
		totalSectors--;
		ibyte += 512;

	}

	spiTransferByte(STOP_TRAN_TOKEN); // send 'stop transmission token'

	retry = 0;
	do {
		res = spiTransferByte(0xFF); // wait for SD card to complete writing and get idle
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_READY_STATE);

	unselectSD();
	spiTransferByte(0xFF); // just spend 8 clock cycle delay before reasserting the CS signal
	selectSD(); // re-assertion of the CS signal is required to verify if card is still busy

	retry = 0;
	do {
		res = spiTransferByte(0xFF); // wait for SD card to complete writing and get idle
		retry++;
		if (retry > NTRIES) goto fail;
	} while(res != R1_READY_STATE);
	unselectSD();

	#ifdef SD_DEBUG
	rprintf("Write Multiple Block Success!!!\n");
	#endif
	return 0;

	fail:
		res = -1;
	fail_res:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Write Multiple Block Fail!!!\n");
		#endif
		return res;
}

uint8_t sdErase(uint32_t startSector, uint32_t totalSectors) 
{
	uint8_t res;

	selectSD();
	res = sdSendCommand(ERASE_BLOCK_START_ADDR , startSector); // send starting block address
	if (res != R1_READY_STATE) {
		#ifdef SD_DEBUG
		rprintf("ERASE_BLOCK_START_ADDR -> 0x%x\n", res);
		#endif
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	res = sdSendCommand(ERASE_BLOCK_END_ADDR , startSector+totalSectors-1); // send end block address
	if (res != R1_READY_STATE) {
		#ifdef SD_DEBUG
		rprintf("ERASE_BLOCK_END_ADDR -> 0x%x\n", res);
		#endif
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	res = sdSendCommand(ERASE_SELECTED_BLOCKS , 0); // erase all selected blocks
	if (res != R1_READY_STATE) {
		#ifdef SD_DEBUG
		rprintf("ERASE_SELECTED_BLOCKS -> 0x%x\n", res);
		#endif
		goto fail_res; // check for SD status: 0x00 - OK (No flags set)
	}
	unselectSD();

	#ifdef SD_DEBUG
	rprintf("Erase Success!!!\n");
	#endif
	return 0;

	fail_res:
		unselectSD();
		#ifdef SD_DEBUG
		rprintf("Erase Fail!!!\n");
		#endif
		return res;
}

uint8_t sdSendCommand(uint8_t cmd, uint32_t arg)
{
	uint8_t res;
	uint8_t retry=0;

	// wait some clock cycles
	spiTransferByte(0xFF);
	// SD card accepts byte address while SDHC accepts block address 
	// in multiples of 512
	// so, if it's card we need to convert block address into corresponding byte
	// address by multiplying it with 512. which is equivalent to shifting left
	// 9 times following 'if' loop does that
	if (sdCardType != SD_CARD_TYPE_SDHC) {
		if (cmd == READ_SINGLE_BLOCK     ||
			cmd == READ_MULTIPLE_BLOCKS  ||
			cmd == WRITE_SINGLE_BLOCK    ||
			cmd == WRITE_MULTIPLE_BLOCKS ||
			cmd == ERASE_BLOCK_START_ADDR||
			cmd == ERASE_BLOCK_END_ADDR ) arg <<= 9;
	}

	// send command via SPI
	spiTransferByte(0x40 | cmd);
	spiTransferByte((arg>>24) & 0xFF);
	spiTransferByte((arg>>16) & 0xFF);
	spiTransferByte((arg>>8) & 0xFF);
	spiTransferByte(arg & 0xFF);

	switch (cmd)
	{
		case GO_IDLE_STATE:
			spiTransferByte(0x95);
			break;
		case SEND_IF_COND:
			spiTransferByte(0x87);
			break;
		default:
			spiTransferByte(0xFF);
			break;
	}

	// receive response
	while ( ((res=spiTransferByte(0xFF))|0x20) == 0xFF)
		if (retry++ > NTRIES) break;

	//return response
	return res;
}

