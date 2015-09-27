#include <inttypes.h>

#include "nic/w5100/socket.h"
#include "nic/w5100/w5100.h"
#include "nic.h"
#include "nicconf.h"

void nicInit(void)
{
	uint8_t macaddr[6];

	W51_init();
	macaddr[0] = ETHADDR0;
	macaddr[1] = ETHADDR1;
	macaddr[2] = ETHADDR2;
	macaddr[3] = ETHADDR3;
	macaddr[4] = ETHADDR4;
	macaddr[5] = ETHADDR5;
	nicSetMacAddress(macaddr);
	socket(MACRAW_SOCKET, W5100_SKT_MR_MACRAW, 0x00, 0x00);
}

void nicSend(unsigned int len, unsigned char* packet)
{
	macraw_send(packet, len, 0, 0);
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
