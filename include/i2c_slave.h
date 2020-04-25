#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <c_types.h>

void i2c_slave_handle_interrupt(uint32 gpio_status);

void i2c_slave_write(const char *input);

bool i2c_slave_check_address(int address);

void i2c_slave_set_own_address(int address);

void i2c_slave_init();

#endif //I2C_SLAVE_H
