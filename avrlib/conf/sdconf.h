#ifndef SDCONF_H
#define SDCONF_H

// define to enable debugging print statements
#define SD_DEBUG

#define SD_CS_PORT		PORTD
#define	SD_CS_DDR		DDRD
#define SD_CS_PIN		4

#define selectSD()		cbi(SD_CS_PORT, SD_CS_PIN)
#define unselectSD()	sbi(SD_CS_PORT, SD_CS_PIN)

#endif
