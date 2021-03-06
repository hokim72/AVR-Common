#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>

#include "uart.h"
#include "rprintf.h"

#include "uip/timer.h"
#include "uip/uip_arp.h"
#include "nic/network.h"
#include "nic/enc28j60/enc28j60.h"
#include "nicconf.h"


#include <string.h>
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

volatile uint16_t mytimer;

int main(void)
{


	network_init();

	//CLKPR = (1<<CLKPCE);	//Change prescaler
	//CLKPR = (1<<CLKPS0);	//Use prescaler 2
	//clock_prescale_set(clock_div_2);
	enc28j60Write(ECOCON, 1 & 0x7);	//Get a 25MHz signal from enc28j60

	uartInit();
	uartSetBaudRate(9600);
	rprintfInit(uartSendByte);

	int i;
	uip_ipaddr_t ipaddr; 
	struct timer periodic_timer, arp_timer;

	clock_init();

	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);


	uip_init();
	
	struct uip_eth_addr mac = {UIP_ETHADDR0, UIP_ETHADDR1, UIP_ETHADDR2, UIP_ETHADDR3, UIP_ETHADDR4, UIP_ETHADDR5};

	uip_setethaddr(mac);
	
	// set up the udp data stream
	// configure the target system ip and port
	uip_ipaddr_t target_ipaddr;
	uip_ipaddr(&target_ipaddr, 192,168,0,10);
	udpds_conf(&target_ipaddr, 1000, 1);

#ifdef __DHCPC_H__
	dhcpc_init(&mac, 6);
#else
    uip_ipaddr(ipaddr, 192,168,0,1);
	uip_sethostaddr(ipaddr); 
    uip_ipaddr(ipaddr, 192,168,0,1);
    uip_setdraddr(ipaddr);
    uip_ipaddr(ipaddr, 255,255,255,0);
    uip_setnetmask(ipaddr);
#endif /*__DHCPC_H__*/


	while(1){
		uip_len = network_read(); 
		if(uip_len > 0) {
			if(BUF->type == htons(UIP_ETHTYPE_IP)){  
				uip_arp_ipin();
				uip_input();
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}

			}else if(BUF->type == htons(UIP_ETHTYPE_ARP)){
				uip_arp_arpin();
				if(uip_len > 0){
					network_send();
				}
			}

		}else if(timer_expired(&periodic_timer)) {
			timer_reset(&periodic_timer);
			for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
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

int udpds_set_payload(void)
{
	return (1 + sprintf((char *)uip_appdata, "mytimer = %d\n", mytimer++));
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

