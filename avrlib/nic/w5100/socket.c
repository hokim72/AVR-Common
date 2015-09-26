#include "w5100.h"

#include "socket.h"

static uint16_t local_port;

uint8_t socket(uint8_t s, uint8_t protocol, uint16_t port, uint8_t flag)
{
  	uint8_t ret;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (
	  (protocol == W5100_SKT_MR_TCP) ||
	  (protocol == W5100_SKT_MR_UDP) ||
	  (protocol == W5100_SKT_MR_IPRAW ) ||
	  (protocol == W5100_SKT_MR_MACRAW) ||
	  (protocol == W5100_SKT_MR_PPPOE)
	)
	{
		//if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_CLOSED)
		close(s);
		W51_write(sockaddr+W5100_MR_OFFSET ,protocol|flag);
		if (port != 0) {
			W51_write(sockaddr+W5100_PORT_OFFSET, (uint8_t)((port & 0xFF00) >> 8 ));
			W51_write(sockaddr+W5100_PORT_OFFSET + 1, (uint8_t)(port & 0x00FF));
		}
		else
		{
			local_port++; // if don't set the source port, set local_port number.
			W51_write(sockaddr+W5100_PORT_OFFSET, (uint8_t)((local_port & 0xFF00) >> 8 ));
			W51_write(sockaddr+W5100_PORT_OFFSET + 1, (uint8_t)(local_port & 0x00FF));
		}
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_OPEN);
		
		// wait to process the command...
		while ( W51_read(sockaddr+W5100_CR_OFFSET)) ;

		switch (protocol)
		{
			case W5100_SKT_MR_TCP:
				// wait to achieve the command...
				while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_INIT);
				break;
			case W5100_SKT_MR_UDP:
				// wait to achieve the command...
				while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_UDP);
				break;
			case W5100_SKT_MR_IPRAW:
				// wait to achieve the command...
				while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_IPRAW);
				break;
			case W5100_SKT_MR_MACRAW:
				// wait to achieve the command...
				while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_MACRAW);
				break;
			case W5100_SKT_MR_PPPOE:
				// wait to achieve the command...
				while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_PPPOE);
				break;
			default:
				break;
		}
		ret = 1;
	}
	else
	{
		ret = 0;
	}

	return ret;
}

void close(uint8_t s)
{
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CLOSE);

	// wait to process the command...
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;

	// wait to achieve the right state
	while (W51_read(sockaddr+W5100_SR_OFFSET) !=  W5100_SKT_SR_CLOSED) ;

	// all clear
	W51_write(sockaddr+W5100_IR_OFFSET, 0xFF);
}

uint8_t listen(uint8_t s)
{
	uint8_t ret;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_INIT)
	{
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_LISTEN);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET))  ;

		// wait to action the commad...
		while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_LISTEN) ;

		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}

uint8_t connect(uint8_t s, uint8_t* addr, uint16_t port)
{
	uint8_t ret;
	//uint16_t i;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if ( ((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		 ((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		 (port == 0x00)
	   )
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		// set destination IP
		W51_write(sockaddr+W5100_DIPR_OFFSET, addr[0]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+1, addr[1]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+2, addr[2]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+3, addr[3]);
		W51_write(sockaddr+W5100_DPORT_OFFSET, (uint8_t)((port & 0xFF00) >> 8));
		W51_write(sockaddr+W5100_DPORT_OFFSET+1, (uint8_t)(port & 0xFF00));

		//setSUBR(); // set the subnet mask register because of the ARP errata
		//for (i=0; i<4; i++)
		//	W5100_write(W5100_SUBR+i, subnet[i]);

		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CONNECT);

		// wait for completion
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

		while (W51_read(sockaddr+W5100_SR_OFFSET) != W5100_SKT_SR_SYNSENT)
		{
			if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_ESTABLISHED)
			{
				break;
			}
			if (W51_read(sockaddr+ W5100_IR_OFFSET) & W5100_SKT_IR_TIMEOUT) // Sn_IR_TIMEOUT
			{
				W51_write(sockaddr+ W5100_IR_OFFSET, W5100_SKT_IR_TIMEOUT); // clear TIMEOUT Interrupt
				ret = 0;
				break;
			}
		}
		//clearSUBR(); // clear the subnet mask again and keep it because of the ARP errata
		//for (i=0; i<4; i++)
		//	W5100_write(W5100_SUBR+i, 0x00);
	}

	return ret;
}

void disconnect(uint8_t s)
{
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);
	
	W51_write(sockaddr+ W5100_CR_OFFSET, W5100_SKT_CR_DISCON);

	// wait to process the command...
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;

	// wait to action the command...
	while (W51_read(sockaddr+W5100_SR_OFFSET) !=  W5100_SKT_SR_CLOSED)  ;
}

uint16_t send(uint8_t s, uint8_t* buf, uint16_t len)
{
	uint8_t status =0;
	uint16_t ret = 0;
	uint16_t freesize1 = 0;
	uint16_t freesize = 0;
	uint16_t txrd, txrd_before_send;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (len > W5100_TX_BUF_SIZE) 
		ret = W5100_TX_BUF_SIZE; // check size not to exceed MAX size.
	else
		ret = len;

	do
	{
		do {
			freesize1 =  W51_read(sockaddr+W5100_TX_FSR_OFFSET);
			freesize1 = ((freesize1 & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1);
			if (freesize1 != 0)
			{
				freesize =  W51_read(sockaddr+W5100_TX_FSR_OFFSET);
				freesize = ((freesize & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1);
			}
		} while (freesize != freesize1);
		status = W51_read(sockaddr+W5100_SR_OFFSET);
		if ((status != W5100_SKT_SR_ESTABLISHED) && (status != W5100_SKT_SR_CLOSE_WAIT))
		{
			ret = 0;
			break;
		}
	} while (freesize < ret);
	
	// copy data
	W51_send_data_processing(s, (uint8_t *)buf, ret);

	if (ret != 0)
	{
		txrd_before_send = W51_read(sockaddr+W5100_TX_RR_OFFSET);
		txrd_before_send = (txrd_before_send << 8) + W51_read(sockaddr+W5100_TX_RR_OFFSET+1);

		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_SEND);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

		while ((W51_read(sockaddr+W5100_IR_OFFSET) & W5100_SKT_IR_SEND_OK) != W5100_SKT_IR_SEND_OK)
		{
			if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_CLOSED)
			{
				close(s);
				return 0;
			}
		}

		W51_write(sockaddr+W5100_IR_OFFSET, W5100_SKT_IR_SEND_OK);

		txrd = W51_read(sockaddr+W5100_TX_RR_OFFSET);
		txrd = (txrd << 8) + W51_read(sockaddr+W5100_TX_RR_OFFSET+1);

		if (txrd > txrd_before_send) {
			ret = txrd - txrd_before_send;
		} else {
			ret = (0xFFFF - txrd_before_send) + txrd + 1;
		}
	}

	return ret;
}

uint16_t recv(uint8_t s, uint8_t* buf, uint16_t len)
{
	uint16_t ret = 0;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (len > 0) 
	{
		W51_recv_data_processing(s, buf, len);
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_RECV);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

		ret = len;
	}
	return ret;
}

uint16_t sendto(uint8_t s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t port)
{
	uint16_t ret = 0;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (len > W5100_TX_BUF_SIZE)
		ret = W5100_TX_BUF_SIZE; // check size not to exceed MAX size.
	else
		ret = len;

	if (
		((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		(port == 0x00)
	   )
	{
		ret = 0;
	}
	else
	{
		W51_write(sockaddr+W5100_DIPR_OFFSET+0, addr[0]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+1, addr[1]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+2, addr[2]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+3, addr[3]);
		W51_write(sockaddr+W5100_DPORT_OFFSET, (uint8_t)((port & 0xFF00) >> 8));
		W51_write(sockaddr+W5100_DPORT_OFFSET+1, (uint8_t)(port & 0x00FF));

		// copy data
		W51_send_data_processing(s, (uint8_t *)buf, ret);

		//setSUBR();
		//for (i=0; i<4; i++)
		//	W5100_write(W5100_SUBR+i, subnet[i]);

		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_SEND);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

		while ((W51_read(sockaddr+W5100_IR_OFFSET) &  W5100_SKT_IR_SEND_OK) !=  W5100_SKT_IR_SEND_OK)
		{
			if (W51_read(sockaddr+W5100_IR_OFFSET) & W5100_SKT_IR_TIMEOUT)
			{
				W51_write(sockaddr+W5100_IR_OFFSET, (W5100_SKT_IR_SEND_OK | W5100_SKT_IR_TIMEOUT)); // clear SEND_OK & TIMEOUT
				return 0;
			}
		}
		//clearSUBR();
		//for (i=0; i<4; i++)
		//	W5100_write(W5100_SUBR+i, 0x00);

		W51_write(sockaddr+W5100_IR_OFFSET, W5100_SKT_IR_SEND_OK);
	}
	return ret;
}

uint16_t recvfrom(uint8_t s, uint8_t* buf, uint16_t len, uint8_t* addr, uint16_t* port)
{
	uint8_t head[8];
	uint16_t data_len = 0;
	uint16_t ptr = 0;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	if (len > 0)
	{
		ptr = W51_read(sockaddr+W5100_RX_RD_OFFSET);
		ptr = ((ptr & 0x00FF) << 8) + W51_read(sockaddr+W5100_RX_RD_OFFSET+1);

		switch (W51_read(sockaddr+W5100_MR_OFFSET) & 0x07)
		{
		case W5100_SKT_MR_UDP:
			W51_read_data(s, (uint8_t *)ptr, head, 0x08);
			ptr += 8;
			// read peer's IP address, port number.
			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];
			*port = head[4];
			*port = (*port << 8) + head[5];
			data_len = head[6];
			data_len = (data_len << 8) + head[7];

			W51_read_data(s, (uint8_t *)ptr, buf, data_len); // data copy
			ptr += data_len;

			W51_write(sockaddr+W5100_RX_RD_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
			W51_write(sockaddr+W5100_RX_RD_OFFSET+1, (uint8_t)(ptr & 0x00FF));
			break;

		case W5100_SKT_MR_IPRAW:
			W51_read_data(s, (uint8_t *)ptr, head, 0x06);
			ptr += 6;

			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];
			data_len = head[4];
			data_len = (data_len << 8) + head[5];

			W51_read_data(s, (uint8_t *)ptr, buf, data_len); // data copy
			ptr += data_len;

			W51_write(sockaddr+W5100_RX_RD_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
			W51_write(sockaddr+W5100_RX_RD_OFFSET+1, (uint8_t)(ptr & 0x00FF));
			break;

		case W5100_SKT_MR_MACRAW:
			W51_read_data(s,(uint8_t*)ptr,head, 0x02);
			ptr += 2;
			data_len = head[0];
			data_len = (data_len << 8) + head[1] - 2;

			if (data_len > 1514)
			{
				break;
			}

			W51_read_data(s,(uint8_t*) ptr,buf,data_len);
			ptr += data_len;
			W51_write(sockaddr+W5100_RX_RD_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
			W51_write(sockaddr+W5100_RX_RD_OFFSET+1, (uint8_t)(ptr & 0x00FF));
			break;

		default:
			break;
		}
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_RECV);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;
	}
	return data_len;
}

uint16_t macraw_send(uint8_t* buf, uint16_t len)
{
	uint16_t ptr;
	uint16_t ret;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(MACRAW_SOCKET);

	if (len >  W5100_TX_BUF_SIZE)
		ret = W5100_TX_BUF_SIZE; // check size not to exceed MAX size.
	else ret = len;

	if (ret == 0)
	{
	}
	else
	{
		ptr = W51_read(sockaddr+W5100_TX_WR_OFFSET);
		ptr = (ptr << 8) + W51_read(sockaddr+W5100_TX_WR_OFFSET+1);

		W51_write_data(MACRAW_SOCKET, buf, (uint8_t *)(ptr), len );
		ptr+= len;

		W51_write(sockaddr+W5100_TX_WR_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
		W51_write(sockaddr+W5100_TX_WR_OFFSET+1, (uint8_t)(ptr & 0x00FF));

		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_SEND);

		// wait to process the command...
		while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

		while ((W51_read(sockaddr+W5100_IR_OFFSET) & W5100_SKT_IR_SEND_OK) != W5100_SKT_IR_SEND_OK)
		{
			if (W51_read(sockaddr+W5100_IR_OFFSET) & W5100_SKT_IR_TIMEOUT)
			{
				return 0;
			}
		}

		W51_write(sockaddr+W5100_IR_OFFSET, W5100_SKT_IR_SEND_OK);
	}

	return ret;
}

uint16_t macraw_recv(uint8_t* buf, uint16_t len)
{
	uint8_t head[2];
	uint16_t data_len = 0;
	uint16_t ptr;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(MACRAW_SOCKET);

	data_len = W51_read(sockaddr+W5100_RX_RSR_OFFSET);
	data_len =  ((data_len & 0x00FF) << 8) + W51_read(sockaddr+W5100_RX_RSR_OFFSET+1);

	if (data_len == 0 ) return data_len;

	if (len > 0)
	{
		ptr = W51_read(sockaddr+W5100_RX_RD_OFFSET);
		ptr = ((ptr & 0x00FF) << 8) + W51_read(sockaddr+W5100_RX_RD_OFFSET+1);

		W51_read_data( MACRAW_SOCKET, (uint8_t*)ptr, head, 2 );
		ptr += 2;
		data_len = head[0];
		data_len = (data_len<<8) + head[1] - 2;

		if (data_len > 1514)
		{
			close(MACRAW_SOCKET);
			socket(MACRAW_SOCKET, W5100_SKT_MR_MACRAW, 0x00, 0x00);

			return 0;
		}

		W51_read_data( MACRAW_SOCKET, (uint8_t*) ptr, buf, data_len );
		ptr += data_len;
		W51_write(sockaddr+W5100_RX_RD_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
		W51_write(sockaddr+W5100_RX_RD_OFFSET+1, (uint8_t)(ptr & 0x00FF));
	}

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_RECV);

	// wait to process the command...
	while (W51_read(sockaddr+W5100_CR_OFFSET)) ;

	return data_len;
}
