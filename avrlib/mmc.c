#include <avr/io.h>

#include "spi.h"
#include "mmc.h"
#include "timer.h"
#ifdef MMC_DEBUG
#include "rprintf.h"
#endif

#define CT_MMC           0x01              // MMC ver 3
#define CT_SD1           0x02              // SD ver 1
#define CT_SD2           0x04              // SD ver 2
#define CT_SDC           (CT_SD1|CT_SD2)   // SD
#define CT_BLOCK         0x08              // Block addressing


static volatile uint16_t tick100Hz;
static volatile uint8_t timer1, timer2; // 100Hz decrement timer
static uint8_t cardType;

// functions

// Set low clock (F_CPU / 64)
static void fclkSlow()
{
	sbi(SPCR, SPR1);
	cbi(SPCR, SPR0);
	sbi(SPSR, SPI2X);
}

// Set low clock (F_CPU / 2)
static void fclkFast()
{
	cbi(SPCR, SPR1);
	cbi(SPCR, SPR0);
	sbi(SPSR, SPI2X);
}

static void timerService(void)
{
	uint8_t n;

	n = timer1;
	if (n) timer1 = --n;
	n = timer2;
	if (n) timer2 = --n;

	outb(OCR2A, inb(OCR2A) + tick100Hz);
}

// Wait for card ready
static int waitReady(  //1:Ready, 0:Timeout,  
	uint32_t wt			// Timeout [ms]
)
{
	uint8_t d;

	timer2 = wt/10;
	do {
		d = spiTransferByte(0xFF);
	}
	while (d != 0xFF && timer2);

	return (d == 0xFF) ? 1 : 0;
}

// Returns R1 resp (bit7==1:Send failed) 
// cmd: Command index; arg: Argument
static uint8_t sendCommand(uint8_t cmd, uint32_t arg)
{
	uint8_t n, res;

	if (cmd & 0x80) { // ACMD<n> is the command sequences of CMD55-CMD<n>
		cmd &= 0x7F;
		res = sendCommand(APP_CMD, 0);
		if (res > 1) return res;
	}

	// Select the card and wait for ready except to stop multiple block read
	//if (cmd != STOP_TRANSMISSION){
	if (cmd != STOP_TRANSMISSION && cmd != GO_IDLE_STATE){
		mmcDeselect();
		if (!mmcSelect()) return 0xFF;
	}


	// Send command packet
	spiTransferByte(0x40 | cmd);		// Start + Command index 
	spiTransferByte((uint8_t)(arg>>24));// Argument[31..24]
	spiTransferByte((uint8_t)(arg>>16));// Argument[23..16]
	spiTransferByte((uint8_t)(arg>>8)); // Argument[15..8]
	spiTransferByte((uint8_t)arg);		// Argument[7..0]
	n = 0x01;						// Dummy CRC + Stop
	if (cmd == GO_IDLE_STATE) n = 0x95;	// Valid CRC for CMD0(0) + Stop 
	if (cmd == SEND_IF_COND) n = 0x87;	// Valid CRC for CMD8(0x1AA) + Stop
	spiTransferByte(n);

	// Receive command response
	if (cmd == STOP_TRANSMISSION) spiTransferByte(0xFF);	// Skip a stuff byte when stop reading
	n = 10;			// Wait for a valid response in timeout of 10 attempts
	do
		res = spiTransferByte(0xFF);
	while ((res & 0x80) && --n);

	return res; // Return with the response value
}

// buf: Data buffer to store received data
// btr: Byte count (must be multiple of 4)
static int receiveDataBlock(uint8_t* buf, uint32_t btr)
{
	uint8_t token;

	timer1 = 20;
	do { // Wait for data packet in timeout of 200ms
		token = spiTransferByte(0xFF);
	} while ((token == 0xFF) && timer1);
	if (token != 0xFE) return 0;	// If not valid data token, return with error
	spiReceiveMulti(buf, btr);	// Receive the data block into buffer
	spiTransferByte(0xFF);		// Discard CRC
	spiTransferByte(0xFF);

	return 1;					// Return with success
}

// buf: 512 byte data block to be transmitted
// token: Data/Stop Token
static int sendDataBlock(uint8_t* buf, uint8_t token)
{
	uint8_t resp;

	if (!waitReady(500)) return 0;

	spiTransferByte(token);			// Send data token
	if (token != 0xFD) { // Is data token 
		spiSendMulti(buf, 512); // Send the data block to the MMC
		spiTransferByte(0xFF);	 // CRC (Dummy)
		spiTransferByte(0xFF);
		resp = spiTransferByte(0xFF); // Receive data response
		if ((resp & 0x1F) != 0x05) // If no accepted, return with error
			return 0;
	}
	return 1;
}

// Deselect the card and release SPI bus
void mmcDeselect(void)

{
	MMC_CS_HIGH(); // Set CS# high
	spiTransferByte(0xFF);	// Dummy clock (force D0 hi-z for multiple slave SPI)
}

// Select the card and wait for ready
int mmcSelect(void) // 1: Successful, 0: Timeout
{
	MMC_CS_LOW();				// Set CS# low
	spiTransferByte(0xFF);	// Dummy clock (force D0 enabled)
	if (waitReady(500)) return 1; // wait for card ready

	mmcDeselect();
	return 0; // Timeout
}

void mmcInit(void)
{
	// initialize SPI interface
	spiInit();
	sbi(MMC_CS_DDR, MMC_CS_PIN);
	sbi(MMC_CS_PORT, MMC_CS_PIN);

	// clock = f/32
	sbi(SPCR, SPR1);
	sbi(SPSR, SPI2X);

	// clock = f/128
	//sbi(SPCR, SPR0);
	//sbi(SPCR, SPR1);

	// enable SPI
	sbi(SPCR, SPE);

	tick100Hz = F_CPU/1024/100;
	timer2SetPrescaler(TIMER_CLK_DIV1024);
	timerAttach(TIMER1OUTCOMPAREA_INT, timerService);
	outb(OCR2A, inb(TCNT2) + tick100Hz);
	sbi(TIMSK1, OCIE2A);
}


uint8_t mmcReset(void)
{
	uint8_t n, cmd, ty, ocr[4];

	for (n=0; n<10; n++)
		sendCommand( GO_IDLE_STATE, 0); // important!!! 
										// make MMC send 0xFF packets

	for (n=0; n<10; n++) { // 80 dummy clocks
		ty = spiTransferByte(0xFF); 
		#ifdef MMC_DEBUG
		rprintf("ty = 0x%x\n", ty);
		#endif
	}

	fclkSlow();

	ty = 0;
	if (sendCommand( GO_IDLE_STATE, 0) == 1) {	// Enter Idle state
		timer1 = 100;							// Initialization timeout of 1000 ms
		if (sendCommand(SEND_IF_COND, 0x1AA) == 1) { // SDv2?
			for (n=0; n<4; n++) ocr[n] = spiTransferByte(0xFF);	// Get trailing return value of R7 resp
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {	// The card can work at vdd range of 2.7 - 3.6V
				while (timer1 && sendCommand(SEND_OP_COND_SDC, 1UL << 30)); // Wait for leaving idle state (ACMD41 with HCS bit)
				if (timer1 && sendCommand(READ_OCR, 0) == 0) { // Check CCS bit in the OCR
					for (n=0; n<4; n++) ocr[n] = spiTransferByte(0xFF);
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // SDv2
				}
			}
		} else {	// SDv1 or MMCv3
			if (sendCommand(SEND_OP_COND_SDC, 0) <= 1) {
				ty = CT_SD1; cmd = SEND_OP_COND_SDC; // SDv1
			} else {
				ty = CT_MMC; cmd = SEND_OP_COND_MMC; // MMCv3
			}
			while (timer1 && sendCommand(cmd, 0));	// Wait for leaving idle state
			if (!timer1 || sendCommand(SET_BLOCK_LEN, 512) != 0) // Set R/W block length to 512
				ty = 0;
		}
	}
	cardType = ty;
	mmcDeselect();
	
	#ifdef MMC_DEBUG
	rprintf("cardType = %d\n", cardType);
	#endif

	if (ty) {		// Initialization succeded
		fclkFast();
		return 0;
	} else { // Initialization failed
		return 1;
	}
}

// buf: Pointer to the data buffer to store read data
// sector: Start sector number (LBA)
// count: Sector count (1..128)
uint8_t mmcRead(uint8_t* buf, uint32_t sector, uint32_t count)
{
	uint8_t cmd;

	if (!(cardType & CT_BLOCK)) sector *= 512; // Convert to address if needed
	
	cmd = count > 1 ? READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK; // CMD18:CMD17
	if (sendCommand(cmd, sector) == 0) {
		do {
			if (!receiveDataBlock(buf, 512)) break;
			buf += 512;
		} while (--count);
		if (cmd == READ_MULTIPLE_BLOCK) sendCommand(STOP_TRANSMISSION, 0); //CMD12
	}
	mmcDeselect();

	#ifdef MMC_DEBUG
	rprintf("read count = %d\n", count);
	#endif

	return count ? 1 : 0;
}

// buf: Pointer to the data to be written
// sector: Start sector number (LBA)
// count: Sector count (1..128)
uint8_t mmcWrite(uint8_t* buf, uint32_t sector, uint32_t count)
{
	if (!(cardType & CT_BLOCK)) sector *= 512; // Convert to byte address if needed

	if (count == 1) {// Single block write
		if ((sendCommand(WRITE_BLOCK, sector) == 0) // CMD24
			&& sendDataBlock(buf, 0xFE))
			count = 0;
	}
	else { // Multiple block write
		if (cardType & CT_SDC) sendCommand(SET_WR_BLK_ERASE_COUNT_SDC, count);
		if (sendCommand(WRITE_MULTIPLE_BLOCK, sector) == 0) { // CMD25
			do {
				if (!sendDataBlock(buf, 0xFC)) break;
				buf += 512;
			} while (--count);
			if (!sendDataBlock(0, 0xFD)) // STOP_TRAN token
				count = 1;
		}
	}
	mmcDeselect();

	#ifdef MMC_DEBUG
	rprintf("write count = %d\n", count);
	#endif

	return count ? 1 : 0;
}
