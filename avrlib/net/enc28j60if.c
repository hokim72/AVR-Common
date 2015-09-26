#include <inttypes.h>
#include <util/delay.h>

#include "global.h"
#include "rprintf.h"
#include "nic/enc28j60/enc28j60.h"
#include "nic.h"

void enc28j60RegDump(void)
{
//  unsigned char macaddr[6];
//  result = ax88796Read(TR);

//  rprintf("Media State: ");
//  if(!(result & AUTOD))
//      rprintf("Autonegotiation\r\n");
//  else if(result & RST_B)
//      rprintf("PHY in Reset   \r\n");
//  else if(!(result & RST_10B))
//      rprintf("10BASE-T       \r\n");
//  else if(!(result & RST_TXB))
//      rprintf("100BASE-T      \r\n");

    rprintf("RevID: 0x%x\r\n", enc28j60Read(EREVID));

    rprintfProgStrM("Cntrl: ECON1 ECON2 ESTAT  EIR  EIE\r\n");
    rprintfProgStrM("         ");
    rprintfu08(enc28j60Read(ECON1));
    rprintfProgStrM("    ");
    rprintfu08(enc28j60Read(ECON2));
    rprintfProgStrM("    ");
    rprintfu08(enc28j60Read(ESTAT));
    rprintfProgStrM("    ");
    rprintfu08(enc28j60Read(EIR));
    rprintfProgStrM("   ");
    rprintfu08(enc28j60Read(EIE));
    rprintfCRLF();

    rprintfProgStrM("MAC  : MACON1  MACON2  MACON3  MACON4  MAC-Address\r\n");
    rprintfProgStrM("        0x");
    rprintfu08(enc28j60Read(MACON1));
    rprintfProgStrM("    0x");
    rprintfu08(enc28j60Read(MACON2));
    rprintfProgStrM("    0x");
    rprintfu08(enc28j60Read(MACON3));
    rprintfProgStrM("    0x");
    rprintfu08(enc28j60Read(MACON4));
    rprintfProgStrM("   ");
    rprintfu08(enc28j60Read(MAADR5));
    rprintfu08(enc28j60Read(MAADR4));
    rprintfu08(enc28j60Read(MAADR3));
    rprintfu08(enc28j60Read(MAADR2));
    rprintfu08(enc28j60Read(MAADR1));
    rprintfu08(enc28j60Read(MAADR0));
    rprintfCRLF();

    rprintfProgStrM("Rx   : ERXST  ERXND  ERXWRPT ERXRDPT ERXFCON EPKTCNT MAMXFL\r\n");
    rprintfProgStrM("       0x");
    rprintfu08(enc28j60Read(ERXSTH));
    rprintfu08(enc28j60Read(ERXSTL));
    rprintfProgStrM(" 0x");
    rprintfu08(enc28j60Read(ERXNDH));
    rprintfu08(enc28j60Read(ERXNDL));
    rprintfProgStrM("  0x");
    rprintfu08(enc28j60Read(ERXWRPTH));
    rprintfu08(enc28j60Read(ERXWRPTL));
    rprintfProgStrM("  0x");
    rprintfu08(enc28j60Read(ERXRDPTH));
    rprintfu08(enc28j60Read(ERXRDPTL));
    rprintfProgStrM("   0x");
    rprintfu08(enc28j60Read(ERXFCON));
    rprintfProgStrM("    0x");
    rprintfu08(enc28j60Read(EPKTCNT));
    rprintfProgStrM("  0x");
    rprintfu08(enc28j60Read(MAMXFLH));
    rprintfu08(enc28j60Read(MAMXFLL));
    rprintfCRLF();

    rprintfProgStrM("Tx   : ETXST  ETXND  MACLCON1 MACLCON2 MAPHSUP\r\n");
    rprintfProgStrM("       0x");
    rprintfu08(enc28j60Read(ETXSTH));
    rprintfu08(enc28j60Read(ETXSTL));
    rprintfProgStrM(" 0x");
    rprintfu08(enc28j60Read(ETXNDH));
    rprintfu08(enc28j60Read(ETXNDL));
    rprintfProgStrM("   0x");
    rprintfu08(enc28j60Read(MACLCON1));
    rprintfProgStrM("     0x");
    rprintfu08(enc28j60Read(MACLCON2));
    rprintfProgStrM("     0x");
    rprintfu08(enc28j60Read(MAPHSUP));
    rprintfCRLF();

    _delay_ms(25);
}


void nicInit(void)
{
	enc28j60Init();
}

void nicSend(unsigned int len, unsigned char* packet)
{
	enc28j60PacketSend(len, packet, 0, 0);
}

unsigned int nicPoll(unsigned int maxlen, unsigned char* packet)
{
	//return enc28j60PacketReceive(maxlen, packet);
	return enc28j60PacketReceive(maxlen, packet)-4; // Ignore Ethernet CRC (4 bytes) 
}

void nicGetMacAddress(uint8_t* macaddr)
{
	// read MAC address registers
	// NOTE: MAC address in ENC28J60 is byte-backward
	*macaddr++ = enc28j60Read(MAADR5);
	*macaddr++ = enc28j60Read(MAADR4);
	*macaddr++ = enc28j60Read(MAADR3);
	*macaddr++ = enc28j60Read(MAADR2);
	*macaddr++ = enc28j60Read(MAADR1);
	*macaddr++ = enc28j60Read(MAADR0);
}

void nicSetMacAddress(uint8_t* macaddr)
{
	// write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	enc28j60Write(MAADR5, *macaddr++);
	enc28j60Write(MAADR4, *macaddr++);
	enc28j60Write(MAADR3, *macaddr++);
	enc28j60Write(MAADR2, *macaddr++);
	enc28j60Write(MAADR1, *macaddr++);
	enc28j60Write(MAADR0, *macaddr++);
}

void nicRegDump(void)
{
	enc28j60RegDump();
}

