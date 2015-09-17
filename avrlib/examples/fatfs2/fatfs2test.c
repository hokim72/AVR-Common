#include "uart.h"
#include "rprintf.h"
#include "diskio.h"
#include "ff.h"
#include "timer.h"
#include "mmc.h"
#include <string.h>
#include <stdio.h>


FATFS FatFs; // Work area (file system object) for logical drive

FRESULT scan_files(char* path);

int main(void)
{
	// initialize out libraries
	// initialize the UART (serial port)
	uartInit();
	uartSetBaudRate(9600);
	// make all rprintf statements use uart for output
	rprintfInit(uartSendByte);
	timerInit();

	// Register work arear to the default drive
	f_mount(&FatFs, "", 0);

	scan_files(".");
	

	return 0;
}


FRESULT scan_files(char* path)
{
	uint8_t res;
	FILINFO fno;
	DIR dir;
	int i;
	char* fn; // This function assumes non-Unicod configuration 
#if _USE_LFN
	static char lfn[_MAX_LFN + 1]; // Buffer to store the LFN
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
#endif
	res = f_opendir(&dir, path);	// Open the directory
	if (res == FR_OK) {
		i = strlen(path);
		for (;;) {
			res = f_readdir(&dir, &fno); // Read a directory item 
			if (res != FR_OK || fno.fname[0] == 0) break; // Break on error or end of dir
			if (fno.fname[0] == '.') continue;
#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			if (fno.fattrib & AM_DIR) {	// It is a directory
				sprintf(&path[i], "/%s", fn);
				res = scan_files(path);
				path[i] = 0;
				if (res != FR_OK) break;
			} else {					// It is a file
				rprintfStr(path);
				rprintfChar('/');
				rprintfStr(fn);
				rprintfCRLF();
			}
		}
		f_closedir(&dir);
	}
	return res;
}
