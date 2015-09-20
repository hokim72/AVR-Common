#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/power.h>

//#define MY_DEBUG
#ifdef MY_DEBUG
#include "uart.h"
#include "rprintf.h"
#include "debug.h"
#endif

#include "uip/timer.h"
#include "nicconf.h"
#include "uip/uip_arp.h"
#include "network.h"
#include "enc28j60.h"


#include <string.h>
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])


int main(void)
{

	network_init();
	// network.c
	// enc28j60Init();
	// enc28j60PhyWrite(PHLCON,0x476);

	//CLKPR = (1<<CLKPCE);	//Change prescaler
	//CLKPR = (1<<CLKPS0);	//Use prescaler 2
	clock_prescale_set(clock_div_2);
	enc28j60Write(ECOCON, 1 & 0x7);	
	// enc28j60.c
	//Get a 25MHz signal from enc28j60

	#ifdef MY_DEBUG
	uartInit();
	uartSetBaudRate(9600);
	rprintfInit(uartSendByte);
	#endif

	int i;
	uip_ipaddr_t ipaddr; // uip.h 
	// typedef u16_t uip_ip4addr_t[2]; 
	// typedef uip_ip4addr_t uip_ipaddr_t;
	struct timer periodic_timer, arp_timer;

	clock_init();


	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);

	uip_init();
	// uip.c

	//uip_arp_init();
	// uip_arp.c
	// must be done or sometimes arp doesn't work
	
	struct uip_eth_addr mac = {UIP_ETHADDR0, UIP_ETHADDR1, UIP_ETHADDR2, UIP_ETHADDR3, UIP_ETHADDR4, UIP_ETHADDR5};

	uip_setethaddr(mac);

	simple_httpd_init();

#ifdef __DHCPC_H__
	dhcpc_init(&mac, 6);
#else
    uip_ipaddr(ipaddr, 192,168,0,1); // uip.h
	uip_sethostaddr(ipaddr); // uip.h
	// #define uip_sethostaddr(addr) uip_ipaddr_copy(uip_hostaddr, (addr))
    uip_ipaddr(ipaddr, 192,168,0,1);
    uip_setdraddr(ipaddr);
	// #define uip_setdraddr(addr) uip_ipaddr_copy(uip_draddr, (addr))
    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
	// #define uip_setnetmask(addr) uip_ipaddr_copy(uip_netmask, (addr))

#endif /*__DHCPC_H__*/


	while(1){

		uip_len = network_read(); 
		// uip.c : u16_t uip_len;
		// network.c : return ((unt16_t) enc28j60PacketReceive(UIP_BUFSIZE, (uint8_t *)uip_buf)); 
		// enc28j60.c : enc28j60PacketReceive
		// uip.c : uint8_t uip_buf[UIP_BUFSIZE+2]; 
		// uipconf.h : UIP_BUFSIZE:300
		if(uip_len > 0) {
			if(BUF->type == htons(UIP_ETHTYPE_IP)){  
			#ifdef MY_DEBUG
			//rprintf("eth in : uip_len = %d, proto = %d\n", uip_len, uip_buf[23]);
			//debugPrintHexTable(uip_len, uip_buf);
			#endif
			// struct uip_eth_hdr {
			//	struct uip_eth_addr dest;
			//	struct uip_eth_addr src;
			//	u16_t type;
			//	};
			// struct uip_eth_addr { // Representation of a 48-bit Ethernet address
			// 	u8_t addr[6];
			// };
			//	http://www.netmanias.com/ko/post/blog/5372/ethernet-ip-tcp-ip/packet-header-ethernet-ip-tcp-ip
			//	UIP_ETHTYPE_IP(0x0800) : IPv4 Packet(ICMP, TCP, UDP)
				uip_arp_ipin();
				#ifdef MY_DEBUG
				//rprintf("ip in : uip_len = %d, proto = %d\n", uip_len, uip_buf[23]);
				//debugPrintHexTable(uip_len, uip_buf+14);
				#endif
				// uip_arp.c
				// #define IPBUF ((struct ethip_hdr *)&uip_buf[0])
				// struct ethip_hdr {
				// 	 struct uip_eth_hdr ethhdr;
				// 	 // IP header
				// 	 u8_t vhl,
				// 	 	tos,
				// 	 	len[2],
				// 	 	ipid[2],
				// 	 	ipoffset[2],
				// 	 	ttl,
				// 	 	proto; // ICMP: 1, TCP: 6, UDP: 17
				// 	 u16_t ipchksum;
				// 	 u16_t srcipaddr[2],
				// 	 destipaddr[2];
				// }
				// if ((IBUF->srcipaddr & uip_netmask) != uip_hostaddr & (uip_netmask)) return;
				// uip_arp_update(IPBUF->srcipaddr, &(IPBUF->ethhdr.src));
				uip_input();
				#ifdef MY_DEBUG
				//rprintf("ip out : uip_len = %d, proto = %d\n", uip_len, uip_buf[23]);
				//debugPrintHexTable(uip_len, uip_buf+14);
				#endif
				// ip out packet
				// eg ICMP 
				// uip_len : 84
				// source ip <-> destination ip
				// type : 8 (Echo (ping) request) -> 0 (Echo (ping) reply)
				// uip.h
				// #define uip_input()        uip_process(UIP_DATA) // UIP_DATA(1) : Tells uIP that there is incoming
				// uip_process : The actual uIP function which does all the work.
				if(uip_len > 0) {
					uip_arp_out();
					#ifdef MY_DEBUG
					//rprintf("ip out : uip_len = %d, proto = %d\n", uip_len, uip_buf[23]);
					//debugPrintHexTable(uip_len, uip_buf);
					#endif
					// Destination MAC Address <=> Source MAC Address
					// w/o Ethernet CRC
					// uip_arp.c

					network_send();
					// network.c
					// if(uip_len <= UIP_LLH_LEN + 40){ // UIP_LLH_LEN : 14
					// 	enc28j60PacketSend(uip_len, (uint8_t *)uip_buf, 0, 0);
					// }else{
					// enc28j60PacketSend(54, (uint8_t *)uip_buf , uip_len - UIP_LLH_LEN - 40, (uint8_t*)uip_appdata);
					// }
				}
			}else if(BUF->type == htons(UIP_ETHTYPE_ARP)){
				// UIP_ETHYPE_ARP(0x0806)
				#ifdef MY_DEBUG
				//rprintf("arp in : uip_len = %d\n", uip_len);
				//debugPrintHexTable(uip_len, uip_buf);
				#endif
				// arp in : 64 bytes
				uip_arp_arpin();
				if(uip_len > 0){ // always uip_len > 0
					#ifdef MY_DEBUG
					//rprintf("ip in : uip_len = %d\n", uip_len);
					//debugPrintHexTable(uip_len, uip_buf);
					#endif
					// arp out : 42 bytes
					// 64 - Ethernet_padding(18) - Ethernet_CRC(4)
					// Send MAC address <--> Target MAC address
					// Send IP address <--> Target IP address
					// uip_arp.c
					network_send();
				}
			}

		}else if(timer_expired(&periodic_timer)) {

			timer_reset(&periodic_timer);

			for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
                // uip.h
                // #define uip_udp_periodic(conn) do { uip_udp_conn = &uip_udp_conns[conn]; \
                // uip_process(UIP_UDP_TIMER); } while (0) // UIP_UDP_TIMER : 5
                // uip.c; uip_process
                // if(flag == UIP_UDP_TIMER) {
                //  if(uip_udp_conn->lport != 0) {
                //      uip_conn = NULL;
                //      uip_sappdata = uip_appdata = &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
                //      uip_len = uip_slen = 0;
                //      uip_flags = UIP_POLL;
                //      UIP_UDP_APPCALL();
                //      goto udp_send;
                //  }
                // } else {
                //  goto drop;
                // }
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}

			#if UIP_UDP
			for(i = 0; i < UIP_UDP_CONNS; i++) {
				uip_udp_periodic(i);
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}
			#endif /* UIP_UDP */

			if(timer_expired(&arp_timer)) {
				timer_reset(&arp_timer);
				uip_arp_timer();
			}
		}
	}
	return 0;
}

#ifdef __DHCPC_H__
void dhcpc_configured(const struct dhcpc_state *s)
{
	#ifdef MY_DEBUG
	//rprintf("IP : %d.%d.%d.%d\n", uip_ipaddr1(s->ipaddr), uip_ipaddr2(s->ipaddr), uip_ipaddr3(s->ipaddr), uip_ipaddr4(s->ipaddr));
	//rprintf("NM : %d.%d.%d.%d\n", uip_ipaddr1(s->netmask), uip_ipaddr2(s->netmask), uip_ipaddr3(s->netmask), uip_ipaddr4(s->netmask));
	//rprintf("GW : %d.%d.%d.%d\n", uip_ipaddr1(s->default_router), uip_ipaddr2(s->default_router), uip_ipaddr3(s->default_router), uip_ipaddr4(s->default_router));
	#endif

    uip_sethostaddr(s->ipaddr);
    uip_setnetmask(s->netmask);
    uip_setdraddr(s->default_router);
}
#endif /* __DHCPC_H__ */


