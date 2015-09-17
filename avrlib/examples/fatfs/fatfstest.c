#include "uart.h"
#include "rprintf.h"
#include "diskio.h"
#include "ff.h"
#include "timer.h"
#include "mmc.h"

int main(void)
{

	FATFS fs; // Work area (file system object) for logical drive
	FIL fil; // File object
	uint8_t res; // API result
	uint16_t bw; // Bytes written
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	timerInit();

	// Register work area (do not care about error)
	f_mount(&fs, "", 0);

	// Create FAT volume with default cluster size
	f_mkfs("", 0, 0);

	// Create a file as new
	f_open(&fil, "hello.txt", FA_CREATE_NEW | FA_WRITE);

	// Write a message
	f_write(&fil, "Hello, World!\r\n", 15, &bw);
	
	// Close the file
	f_close(&fil);

	// Unregister work area
	f_mount(0, "", 0);

	return 0;
}
