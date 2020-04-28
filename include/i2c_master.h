#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include "ring_buffer.h"

extern ring_buffer_t i2c_master_send_buffer;
extern ring_buffer_t i2c_master_receive_buffer;

void i2c_master_timer();

//creates master interface
void i2c_master_init();

//reads from slave
void i2c_master_read(int length);

//writes to slave
void i2c_master_write(uint8 *data);

//writes to slave
void i2c_master_write_byte(uint8 data);

//sets the address where messages will be send to (Default address: 0000000)
void i2c_master_set_target_address(int address);


#endif //I2C_MASTER_H