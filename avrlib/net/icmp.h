// ICMP (Internet Control Message Protocol) has many functions on the internet,
// including the handling of ECHO (ping) request, relaying network route status,
// passing connection status messages, etc.
//
// This library currently handles only ICMP ECHO requests (ping), but may be
// expanded to include other useful ICMP operations as needed.

#ifndef ICMP_H
#define ICMP_H

#include "global.h"
#include "net.h"

//#define ICMP_DEBUG_PRINT

// Initialize ICMP protocol library.
void icmpInit(void);

// Incoming IP packets of protocol ICMP should be passed to this function.
void icmpIpIn(icmpip_hdr* packet);

// Forms and sends a reply in response to an ICMP ECHO request.
void icmpEchoRequest(icmpip_hdr* packet);

// Print ICMP packet information
void icmpPrintHeader(icmpip_hdr* packet);

#endif
