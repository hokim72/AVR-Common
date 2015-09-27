#include "nicconf.h"
#include <avr/io.h>
#include <util/delay.h>
#include "global.h"

#include "w5100.h"
//#define MY_DEBUG
#ifdef MY_DEBUG
#include "rprintf.h"
#endif

void W51_config(W51_CFG* pcfg)
{
	W51_write(W5100_SHAR, pcfg->macaddr[0]);	// set up the MAC address
	W51_write(W5100_SHAR+1, pcfg->macaddr[1]);
	W51_write(W5100_SHAR+2, pcfg->macaddr[2]);
	W51_write(W5100_SHAR+3, pcfg->macaddr[3]);
	W51_write(W5100_SHAR+4, pcfg->macaddr[4]);
	W51_write(W5100_SHAR+5, pcfg->macaddr[5]);

	W51_write(W5100_SIPR, pcfg->ipaddr[0]);	// set up the source IP address
	W51_write(W5100_SIPR+1, pcfg->ipaddr[1]);	
	W51_write(W5100_SIPR+2, pcfg->ipaddr[2]);	
	W51_write(W5100_SIPR+3, pcfg->ipaddr[3]);	

	W51_write(W5100_SUBR, pcfg->submask[0]);	// set up the subnet mask
	W51_write(W5100_SUBR+1, pcfg->submask[1]);	
	W51_write(W5100_SUBR+2, pcfg->submask[2]);	
	W51_write(W5100_SUBR+3, pcfg->submask[3]);	

	W51_write(W5100_GAR, pcfg->gwaddr[0]);	// set up the gateway address
	W51_write(W5100_GAR+1, pcfg->gwaddr[1]);	
	W51_write(W5100_GAR+2, pcfg->gwaddr[2]);	
	W51_write(W5100_GAR+3, pcfg->gwaddr[3]);	
}

void W51_init(void)
{
	#ifdef MY_DEBUG
	uint8_t ret;
	#endif

	// initialize I/O
	sbi(W5100_CONTROL_DDR, W5100_CONTROL_CS);
	sbi(W5100_CONTROL_PORT, W5100_CONTROL_CS);

	// setup SPI I/O pins
	sbi(W5100_SPI_PORT, W5100_SPI_SCK);			// set SCK hi
	sbi(W5100_SPI_DDR, W5100_SPI_SCK);			// set SCK as output
	cbi(W5100_SPI_DDR, W5100_SPI_MISO);			// set MISO as input
	sbi(W5100_SPI_DDR, W5100_SPI_MOSI);			// set MOSI as output
	sbi(W5100_SPI_DDR, W5100_SPI_SS);			// SS must be output for Master mod to work

	// initialize SPI interface
	// master mode
	sbi(SPCR, MSTR);
	// select clock phase positive-going in middle of data
	cbi(SPCR, CPOL);
	cbi(SPCR, CPHA);
	// Data order MSB first
	cbi(SPCR, DORD);
	// switch to f/4 2X = f/2 bitrate
	cbi(SPCR, SPR0);
	cbi(SPCR, SPR1);
	sbi(SPSR, SPI2X);
	// enable SPI
	sbi(SPCR, SPE);

	W51_write(W5100_MR, W5100_MR_SOFTRST);
	_delay_ms(1);

	#ifdef MY_DEBUG
	ret = W51_read(W5100_MR);
	rprintfProgStrM("MR = 0x"); rprintfu08(ret); rprintfCRLF();
	#endif

	W51_write(W5100_RMSR, 0x55);                    // use default buffer sizes (2K bytes RX and TX for each socket

	W51_write(W5100_TMSR, 0x55);
}

uint8_t W51_read(uint16_t addr)
{
	uint8_t data;

	// assert CS
	cbi(W5100_CONTROL_PORT, W5100_CONTROL_CS);

	SPDR = W5100_READ_OPCODE;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = (addr & 0xFF00) >> 8;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = addr & 0x00FF;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = 0x00;
	loop_until_bit_is_set(SPSR, SPIF);
	data = SPDR;

	// release CS
	sbi(W5100_CONTROL_PORT, W5100_CONTROL_CS);

	return data;
}

void W51_write(uint16_t addr, uint8_t data)
{
	// assert CS
	cbi(W5100_CONTROL_PORT, W5100_CONTROL_CS);

	SPDR = W5100_WRITE_OPCODE;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = (addr & 0xFF00) >> 8;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = addr & 0x00FF;
	loop_until_bit_is_set(SPSR, SPIF);
	SPDR = data;
	loop_until_bit_is_set(SPSR, SPIF);

	// release CS
	sbi(W5100_CONTROL_PORT, W5100_CONTROL_CS);
}

void W51_read_data(uint16_t s, volatile uint8_t* src, volatile uint8_t* dst, uint16_t len)
{
	uint16_t size;
	uint16_t i;
	uint16_t src_mask;
	uint8_t* src_ptr;

	src_mask = (uint16_t)src & W5100_RX_BUF_MASK;
	src_ptr = (uint8_t*)(W5100_RX_BUF_BASE(s) + src_mask);

	if (src_mask+len > W5100_RX_BUF_SIZE)
	{
		size = W5100_RX_BUF_SIZE - src_mask;
		for (i=0; i<size; i++)
			dst[i] = W51_read((uint16_t)src_ptr+i);
		dst += size;
		size = len - size;
		src_ptr = (uint8_t*)W5100_RX_BUF_BASE(s);
		for (i=0; i<size; i++)
			dst[i] = W51_read((uint16_t)src_ptr+i);
	}
	else
	{
		for (i=0; i<len; i++)
			dst[i] = W51_read((uint16_t)src_ptr+i);
	}
}

void W51_write_data(uint8_t s, volatile uint8_t * src, volatile uint8_t * dst, uint16_t len)
{
    uint16_t size;
	uint16_t i;
    uint16_t dst_mask;
    uint8_t * dst_ptr;

    dst_mask = (uint16_t)dst & W5100_TX_BUF_MASK; 
    dst_ptr = (uint8_t *)(W5100_TX_BUF_BASE(s) + dst_mask);


    if (dst_mask + len > W5100_TX_BUF_SIZE)
    {
        size = W5100_TX_BUF_SIZE - dst_mask;
		for (i=0; i<size; i++) 
			W51_write((uint16_t)dst_ptr+i, src[i]);
        src += size;
        size = len - size;
        dst_ptr = (uint8_t *)(W5100_TX_BUF_BASE(s));
		for (i=0; i<size; i++) 
			W51_write((uint16_t)dst_ptr+i, src[i]);
    }
    else
    {
		for (i=0; i<len; i++) 
			W51_write((uint16_t)dst_ptr+i, src[i]);
    }
}

void W51_send_data_processing(uint8_t s, uint8_t *data, uint16_t len)
{
	uint16_t ptr;
    uint16_t sockaddr;

    sockaddr = W5100_SKT_BASE(s);
    
    ptr = W51_read(sockaddr+W5100_TX_WR_OFFSET); // xxx Wrap this up in above test. Save a few read cycles.
    ptr = ((ptr & 0x00FF) << 8) + W51_read(sockaddr+W5100_TX_WR_OFFSET+1);


    W51_write_data(s, data, (uint8_t *)(ptr), len);
    ptr += len;

    W51_write(sockaddr+W5100_TX_WR_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
    W51_write(sockaddr+W5100_TX_WR_OFFSET+1, (uint8_t)(ptr & 0x00FF));
}

void W51_recv_data_processing(uint8_t s, uint8_t* data, uint16_t len)
{
	uint16_t ptr;
	uint16_t sockaddr;

	sockaddr = W5100_SKT_BASE(s);

	ptr = W51_read(sockaddr+W5100_RX_RD_OFFSET);
	ptr = ((ptr & 0x00FF) << 8) + W51_read(sockaddr+W5100_RX_RD_OFFSET+1);

	W51_read_data(s, (uint8_t *)ptr, data, len); // read data
	ptr += len;
	W51_write(sockaddr+W5100_RX_RD_OFFSET, (uint8_t)((ptr & 0xFF00) >> 8));
	W51_write(sockaddr+W5100_RX_RD_OFFSET+1, (uint8_t)(ptr & 0x00FF));
}


