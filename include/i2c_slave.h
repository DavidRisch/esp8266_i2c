#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <c_types.h>

void i2c_slave_handle(uint32 gpio_status);

int i2c_slave_write(const char *input);

int i2c_slave_check_address(int address);

void i2c_slave_set_address(int address);

void i2c_slave_init();

#endif //I2C_SLAVE_H
