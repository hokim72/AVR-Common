#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "uart.h"
#include "rprintf.h"
#include "debug.h"

#include "nicconf.h"
#include "nic/w5100/w5100.h"
#include "nic/w5100/socket.h"

W51_CFG my_cfg =
{
	{ETHADDR0,ETHADDR1,ETHADDR2,ETHADDR3,ETHADDR4,ETHADDR5},
	{192,168,0,1},
	{255,255,255,0},
	{192,168,0,1}
};

int main(void)
{
	uint8_t retval;
	uint8_t myMAC[6];
	uint8_t myIp[4];
	uint8_t mysocket;
	uint16_t sockaddr;
	uint16_t rsize;
	uint8_t buf[512];

	uartInit();
	uartSetBaudRate(9600);
	rprintfInit(uartSendByte);

	W51_init();
	W51_config(&my_cfg);

	retval = W51_read(W5100_MR);
	rprintf("MR=0x%x\n", retval);

	myMAC[0] = W51_read(W5100_SHAR + 0);
	myMAC[1] = W51_read(W5100_SHAR + 1);
	myMAC[2] = W51_read(W5100_SHAR + 2);
	myMAC[3] = W51_read(W5100_SHAR + 3);
	myMAC[4] = W51_read(W5100_SHAR + 4);
	myMAC[5] = W51_read(W5100_SHAR + 5);
	rprintfu08(myMAC[0]); rprintfProgStrM(":");
	rprintfu08(myMAC[1]); rprintfProgStrM(":");
	rprintfu08(myMAC[2]); rprintfProgStrM(":");
	rprintfu08(myMAC[3]); rprintfProgStrM(":");
	rprintfu08(myMAC[4]); rprintfProgStrM(":");
	rprintfu08(myMAC[5]); rprintfCRLF();

	myIp[0] = W51_read(W5100_SIPR + 0);
	myIp[1] = W51_read(W5100_SIPR + 1);
	myIp[2] = W51_read(W5100_SIPR + 2);
	myIp[3] = W51_read(W5100_SIPR + 3);
	rprintf("%d.%d.%d.%d\n", myIp[0], myIp[1], myIp[2], myIp[3]);

	mysocket = 0;
	sockaddr = W5100_SKT_BASE(mysocket); // calc address of W5100 register set for this socket

	while (1) {
		switch (W51_read(sockaddr+W5100_SR_OFFSET)) // based on current status of socket...
		{
			case W5100_SKT_SR_CLOSED: // if socket is closed...
				if (socket(mysocket, W5100_SKT_MR_TCP, 80, 0)) // if successful opening a socket...
				{
					listen(mysocket);
					_delay_ms(1);
					
				}
			case W5100_SKT_SR_ESTABLISHED:	// if socket connection is established...
				rsize = recvsize(sockaddr); // find out how many bytes
				if (recv(mysocket, buf, rsize) != 0) 
				{
// Add code here to process the payload from the packet.
// For now, we just ignore the payload and send a canned HTML page so the client
// at least knows we are alive.
					strcpy_P((char *)buf, PSTR("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
					strcat_P((char *)buf, PSTR("<html>\r\n<body>\r\n"));
					strcat_P((char *)buf, PSTR("<title>Hyunok's W5100 web server (ATmega328p)</title>\r\n"));
					strcat_P((char *)buf, PSTR("<h2>Hyunok's ATmega328p web server using Wiznet W5100 chip</h2>\r\n"));
					strcat_P((char *)buf, PSTR("<br /><hr>\r\n"));
					if (send(mysocket, buf, strlen((char *)buf)) == 0) break; // just throw out now

					strcpy_P((char *)buf, PSTR("This is part 2 of the page."));
					strcat_P((char *)buf, PSTR("</body>\r\n</html>\r\n"));
					if (send(mysocket, buf, strlen((char *)buf)) == 0)  break; // just throw out
					disconnect(mysocket);
				}
				else // no data yet...
				{
					_delay_us(10);
				}
				break;
			case W5100_SKT_SR_FIN_WAIT:
			case W5100_SKT_SR_CLOSING:
			case W5100_SKT_SR_TIME_WAIT:
			case W5100_SKT_SR_CLOSE_WAIT:
			case W5100_SKT_SR_LAST_ACK:
				close(mysocket);
			default:
				break;
		}
	}

	return 0;
}
