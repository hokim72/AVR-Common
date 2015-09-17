#ifndef I2CCONF_H
#define I2CCONF_H

#define I2C_SCL_PORT	DDRC
#define I2C_SCL_PIN		PC5
#define I2C_SDA_PORT	DDRC
#define I2C_SDA_PIN		PC4

// define I2C data buffer sizes
// These buffers are used in interrupt-driven Master sending and receiving,
// and in slave sending and receiving. They must be large enough to store
// the largest I2C packet you expect to send and receive, respectively.

#define I2C_SEND_DATA_BUFFER_SIZE		0x20
#define I2C_RECEIVE_DATA_BUFFER_SIZE	0x20

#endif
