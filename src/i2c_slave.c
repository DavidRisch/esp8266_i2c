#include "i2c_slave.h"

#include <gpio.h>

#include "gpio_interrupt.h"
#include "gpio_util.h"
#include "pins.h"

extern bool i2c_is_master;

enum state {
    IDLE,
    RECEIVE_ADDRESS,
    RECEIVE_READ_WRITE_BIT,
    DATA,
    ACKNOWLEDGE,
    STOP
};
static enum state i2c_slave_state = IDLE;

static int i2c_slave_address = 0b0000000;

static int bit_counter;
static int addressed;
static int read_write_bit;

void i2c_slave_handle(uint32 gpio_status) {
    switch (i2c_slave_state) {
        case IDLE:
            if (pin_read_value(I2C_SCL) == 1) {
                bit_counter = 0;
                addressed = 0b0000000;
                pin_disable_interrupt(I2C_SDA);
                pin_enable_interrupt(I2C_SCL, GPIO_PIN_INTR_POSEDGE);
                i2c_slave_state++;
            }
            break;
        case RECEIVE_ADDRESS:
            bit_counter++;
            if (bit_counter == 7) {
                bit_counter = 0;
                addressed = (addressed | pin_i2c_read(I2C_SDA));
                if (i2c_slave_check_address(addressed)) {
                    i2c_slave_state++;
                } else {
                    pin_disable_interrupt(I2C_SCL);
                    pin_enable_interrupt(I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
                    i2c_slave_state = IDLE;
                }
            } else {
                addressed = (addressed | pin_i2c_read(I2C_SDA)) << 1;
            }
            break;
        case RECEIVE_READ_WRITE_BIT:
            read_write_bit = pin_i2c_read(I2C_SDA);
            i2c_slave_state++;
            break;
        case DATA:
            bit_counter++;
            if (bit_counter == 8) {
                bit_counter = 0;
                i2c_slave_state++;
            }
            break;
        case ACKNOWLEDGE:
            bit_counter++;
            if (bit_counter == 2) {
                if ((((gpio_status >> I2C_SDA) & 1) == 1) && (pin_read_value(I2C_SCL) == 1)) {
                    pin_disable_interrupt(I2C_SCL);
                    pin_enable_interrupt(I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
                    i2c_slave_state++;
                } else {
                    bit_counter = 0;
                    pin_disable_interrupt(I2C_SDA);
                    i2c_slave_state = DATA;
                }
            } else {
                pin_i2c_write(I2C_SDA, 1);
                pin_enable_interrupt(I2C_SDA, GPIO_PIN_INTR_POSEDGE);
            }
            break;
        case STOP:
            // TODO: warning: enumeration value 'STOP' not handled in switch [-Wswitch]
            break;
    }
}

int i2c_slave_write(const char *input) {
    int index = 0;
    while (input[index]) {
        int bit;
        for (bit = 8; bit > -1; bit--) {
            int value = (input[index] >> bit) & 1;
            while (pin_get_current_value(I2C_SCL) == 1) {}
            pin_set_value(I2C_SDA, value);
        }
        index++;
    }
    while (pin_get_current_value(I2C_SCL) == 1) {}
    pin_set_value(I2C_SDA, 0);
    return index;
    pin_set_output(I2C_SDA);
}

int i2c_slave_check_address(int address) {
    if (address == i2c_slave_address) {
        return 1;
    } else {
        return 0;
    }
}

void i2c_slave_set_address(int address) {
    i2c_slave_address = address;
}

void i2c_slave_init() {
    pin_enable_interrupt(I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
}
