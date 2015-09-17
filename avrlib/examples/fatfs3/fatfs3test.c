#include "uart.h"
#include "rprintf.h"
#include "diskio.h"
#include "ff.h"
#include "timer.h"
#include <string.h>
#include <stdio.h>


FATFS FatFs; // Work area (file system object) for logical drive

int main(void)
{
	FIL fil; // File object
	char line[82]; // Line buffer
	uint8_t res; // FatFs return code
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	timerInit();

	// Register work arear to the default drive
	f_mount(&FatFs, "", 0);

	// Open a text file
	res = f_open(&fil, "miniterm.py", FA_READ);
	if (res) return (int)res;

	// Read all lines and display it
	while (f_gets(line, sizeof line, &fil))
		rprintfStr(line);

	return 0;
}

