#ifndef MMCCONF_H
#define MMCCONF_H

// define to enable debugging print statements
#define MMC_DEBUG

#define MMC_CS_PORT		PORTD
#define	MMC_CS_DDR		DDRD
#define MMC_CS_PIN		4

#define MMC_CS_HIGH()		sbi(MMC_CS_PORT, MMC_CS_PIN)
#define MMC_CS_LOW()		cbi(MMC_CS_PORT, MMC_CS_PIN)

#endif
