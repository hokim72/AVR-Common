#ifndef SOCKET_H
#define SOCKET_H

#include <inttypes.h>
#include "global.h"

#define MACRAW_SOCKET			0		// THIS IS VERY SPECIAL. Only Socket 0 can do MAC RAW mode.

// This socket function initialize the channel in particular mode, and set the port and wait for W5100 to complete its state change.
// return 1 for success else 0.
// s: for socket number
// protocol: for socket protocol
// port: the source port for the socket
// flag: the option for the socket
uint8_t socket(uint8_t s, uint8_t protocol, uint16_t port, uint8_t flag);

// This function close the socket and parameter is "s" which represent the socket number
void close(uint8_t s);

// This function establishes the connection for the channel in passive (server) mode.
// This function waits for the request from the peer.
// return 1 for success else 0.
// s: the socket number
uint8_t listen(uint8_t s);

// This function established the connection for the channel in Active (client) mode.
// This function waits for the until the connection is established.
// return 1 for success else 0.
uint8_t connect(uint8_t s, uint8_t* addr, uint16_t port);

// This function used for disconnect the socket and parameter is "s" which represent the socket number
void disconnect(uint8_t s);

// This fucntion used to send the data in TCP mode
// return bytes transmitted length for success else 0 for failure.
// s : the socket index
// buf : a pointer to data
// len : the data size to be sent
uint16_t send(uint8_t s, uint8_t* buf, uint16_t len);

uint16_t recvsize(uint16_t sockaddr);

// This function is an application I/F function which is used to receive the data in TCP mode. It continues to wait for data as much as the application wants to receive.
// return received data size for success else 0.
// s: socket index
// buf: a pointer to copy the data to be received.
// len: the data size to be read.
uint16_t recv(uint8_t s, uint8_t* buf, uint16_t len);

// This function is an application I/F function which is used to send the data for other than TCP mode.
// Unlike TCP transmission, The peer's destination address and the port is needed.
// return This function return send data size for success else 0.
// s: socket index
// buf: a pointer to the data
// len: the data size to send.
// addr: the peer's Destination IP address
// port: the peer's destination port number
uint16_t sendto(uint8_t s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t port);

// This function is an application I/F function which is used to receive the data in other than TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well.
// return This function return received data size for success else 0.
// s: the socket number
// buf: a pointer to copy the data to be received.
// len: the data size to read.
// addr: a pointer to store the peer's IP address
// port: a pointer to store the peer's port number
uint16_t recvfrom(uint8_t s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t* port);

// This function is an application I/F function which is used to send the data in MACRAW mode. There is no two byte length header on the send function.
// return This function return send data size for success else 0.
// buf: a pointer to copy the data to be sent.
// len: the data size to read.
uint16_t macraw_send(uint8_t* buf1, uint16_t len1, uint8_t* buf2, uint16_t len2);

// This function is an application I/F function which is used to receive the data MAC_RAW mode, and handle the 2 byte length header as well.
// return This function return received data size for success else 0.
// buf: a pointer to copy the data to be received.
// len: the data size to read.
uint16_t macraw_recv(uint8_t* buf, uint16_t len);


#endif
