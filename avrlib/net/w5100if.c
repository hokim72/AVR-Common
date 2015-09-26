#include <inttypes.h>

#include "global.h"
#include "nic/w5100/socket.h"
#include "nic/w5100/w5100.h"
#include "nic.h"
#include "rprintf.h"
#include "nicconf.h"

void nicInit(void)
{
	uint8_t macAddr[6];
	W51_init();
	macAddr[0] = ETHADDR0;
	macAddr[1] = ETHADDR1;
	macAddr[2] = ETHADDR2;
	macAddr[3] = ETHADDR3;
	macAddr[4] = ETHADDR4;
	macAddr[5] = ETHADDR5;
	nicSetMacAddress(macAddr);
	socket(MACRAW_SOCKET, W5100_SKT_MR_MACRAW, 0x00, 0x00);
}

void nicSend(unsigned int len, unsigned char* packet)
{
	macraw_send(packet, len);
}

unsigned int nicPoll(unsigned int maxlen, unsigned char* packet)
{
	return macraw_recv(packet, maxlen);
}

void nicGetMacAddress(uint8_t* macaddr)
{
	*macaddr++ = W51_read(W5100_SHAR);
	*macaddr++ = W51_read(W5100_SHAR+1);
	*macaddr++ = W51_read(W5100_SHAR+2);
	*macaddr++ = W51_read(W5100_SHAR+3);
	*macaddr++ = W51_read(W5100_SHAR+4);
	*macaddr++ = W51_read(W5100_SHAR+5);
}

void nicSetMacAddress(uint8_t* macaddr)
{
	W51_write(W5100_SHAR, *macaddr++);
	W51_write(W5100_SHAR+1, *macaddr++);
	W51_write(W5100_SHAR+2, *macaddr++);
	W51_write(W5100_SHAR+3, *macaddr++);
	W51_write(W5100_SHAR+4, *macaddr++);
	W51_write(W5100_SHAR+5, *macaddr++);
}
