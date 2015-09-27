#include <inttypes.h>
#include "nicconf.h"
#include "nic/w5100/socket.h"
#include "nic/w5100/w5100.h"

void network_get_MAC(uint8_t* macaddr)
{
	*macaddr++ = W51_read(W5100_SHAR);
	*macaddr++ = W51_read(W5100_SHAR+1);
	*macaddr++ = W51_read(W5100_SHAR+2);
	*macaddr++ = W51_read(W5100_SHAR+3);
	*macaddr++ = W51_read(W5100_SHAR+4);
	*macaddr++ = W51_read(W5100_SHAR+5);
}

void network_set_MAC(uint8_t* macaddr)
{
	W51_write(W5100_SHAR, *macaddr++);
	W51_write(W5100_SHAR+1, *macaddr++);
	W51_write(W5100_SHAR+2, *macaddr++);
	W51_write(W5100_SHAR+3, *macaddr++);
	W51_write(W5100_SHAR+4, *macaddr++);
	W51_write(W5100_SHAR+5, *macaddr++);
}

unsigned int network_read(void)
{
	return macraw_recv((uint8_t*)uip_buf, UIP_BUFSIZE);
}

void network_send(void)
{
	if (uip_len <= UIP_LLH_LEN+40) {
		macraw_send((uint8_t*)uip_buf, uip_len, 0, 0);
	} else {
		macraw_send((uint8_t*)uip_buf, 54, (uint8_t*)uip_appdata, uip_len - UIP_LLH_LEN - 40);
	}
}

void network_init(void)
{
	uint8_t macaddr[6];

	W51_init();
	macaddr[0] = ETHADDR0;
	macaddr[1] = ETHADDR1;
	macaddr[2] = ETHADDR2;
	macaddr[3] = ETHADDR3;
	macaddr[4] = ETHADDR4;
	macaddr[5] = ETHADDR5;
	network_set_MAC(macaddr);
	socket(MACRAW_SOCKET, W5100_SKT_MR_MACRAW, 0x00, 0x00);
}
