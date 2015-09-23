// The IP (Internet Protocol) library provide support for sending IP and 
// IP-related packets. It's not clear if additional features are needed or
// will be added, or even if this is the proper way to facilitate IP packet
// operations.

#ifndef IP_H
#define IP_H

#include "net.h"
#include "arp.h"

#include <inttypes.h>

struct ipConfig					// IP addressing/configuration structure
{
	uint32_t ip;				// IP address
	uint32_t netmask;			// netmask
	uint32_t gateway;			// gateway IP address
};

#define	IP_TIME_TO_LIVE			128		// default Time-To-Live (TTL) value to use in IP headers

// Set our IP address and routing information.
// The myIp value will be used in the source field of IP packets.
void ipSetConfig(uint32_t myIp, uint32_t netmask, uint32_t gatewayIp);

// Get our local IP configuration.
// Returns pointer to current IP address/configuration.
struct ipConfig* ipGetConfig(void);

// Send an IP packet.
void ipSend(uint32_t dstIp, uint8_t protocol, uint16_t len, uint8_t* data);

// Send a UDP/IP packet.
void udpSend(uint32_t dstIp, uint16_t dstPort, uint16_t len, uint8_t* data);

// Print IP configuration
void ipPrintConfig(struct ipConfig* config);

#endif
