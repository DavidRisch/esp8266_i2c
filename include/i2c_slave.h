#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <c_types.h>
#include "ring_buffer.h"

extern ring_buffer_t i2c_slave_receive_buffer;
extern ring_buffer_t i2c_slave_send_buffer;

void i2c_slave_handle_interrupt(uint32 gpio_status, uint32 gpio_values);

void i2c_slave_write(const uint8 *input);

bool i2c_slave_check_address(int address);

void i2c_slave_set_own_address(int address);

void i2c_slave_init();

#endif //I2C_SLAVE_H
