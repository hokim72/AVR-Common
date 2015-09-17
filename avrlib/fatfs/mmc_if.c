/*-----------------------------------------------------------------------*/
/* MMCv3/SDv1/SDv2 (in SPI mode) control module                          */
/*-----------------------------------------------------------------------*/
/*
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <avr/io.h>
#include "diskio.h"
#include "mmc.h"
#include "spi.h"


DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	mmcInit();
	return mmcReset();
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
	return mmcRead(buff, sector, count);
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Sector count (1..128) */
)
{
	return mmcWrite(buff, sector, count);
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD csize;


	if (pdrv) return RES_PARERR;

	res = RES_ERROR;


	switch (cmd) {
	case CTRL_SYNC :		/* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
		if (mmcSelect()) res = RES_OK;
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		if ((mmcSendCommand(SEND_CSD, 0) == 0) && receiveDataBlock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC*/
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if (cardType & CT_SD2) {	/* SDv2? */
			if (mmcSendCommand(SEND_STATUS_SDC, 0) == 0) {	/* Read SD status */
				spiTransferByte(0xFF);
				if (receiveDataBlock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) spiTransferByte(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDv1 or MMCv3 */
			if ((mmcSendCommand(SEND_CSD, 0) == 0) && receiveDataBlock(csd, 16)) {	/* Read CSD */
				if (cardType & CT_SD1) {	/* SDv1 */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMCv3 */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	/* Following commands are never used by FatFs module */

	case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		*ptr = cardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
		if (mmcSendCommand(SEND_CSD, 0) == 0		/* READ_CSD */
			&& receiveDataBlock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
		if (mmcSendCommand(SEND_CID, 0) == 0		/* READ_CID */
			&& receiveDataBlock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
		if (mmcSendCommand(READ_OCR, 0) == 0) {	/* READ_OCR */
			for (n = 4; n; n--) *ptr++ = spiTransferByte(0xFF);
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		if (mmcSendCommand(SEND_STATUS_SDC, 0) == 0) {	/* SD_STATUS */
			spiTransferByte(0xFF);
			if (receiveDataBlock(ptr, 64))
				res = RES_OK;
		}
		break;

	case CTRL_POWER_OFF :	/* Power off */
		res = RES_OK;
		break;

	default:
		res = RES_PARERR;
	}

	mmcDeselect();

	return res;
}
#endif
