
#include <ets_sys.h>
#include <user_interface.h>

#include "pins.h"

void pin_set_output(int pin) {
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1u << pin));
}

void pin_set_input(int pin) {
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) & ~(1u << pin));
}

// pin must be output
void pin_set_value(int pin, int value) {
    if (value) {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) | (1u << pin));
    } else {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) & ~(1u << pin));
    }
}

// pin must be output
int pin_get_current_value(int pin) {
    return (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1u << pin)) > 0;
}


// pin must be input
int pin_read_value(int pin) {
    return (GPIO_REG_READ(GPIO_IN_ADDRESS) & (1u << pin)) > 0;
}

// returns float in range [0, 1)
float pin_read_analog() {
    return ((float) system_adc_read()) / 1024.0f;
}

// write with open drain + pull-up
void pin_i2c_write(volatile uint32 pin, int value) {
    if (value) {
        // use pull-up to write a 1
        pin_set_input(pin);
        PIN_PULLUP_EN(io_mux_address[pin]);
    } else {
        // write 0
        pin_set_output(pin);
        pin_set_value(pin, 0);
    }
}

// read with pull-up
int pin_i2c_read(int pin) {
    pin_i2c_write(pin, 1);
    return pin_read_value(pin);
}

void gpio_util_init() {
    // This functions contains one call to PIN_FUNC_SELECT for each pin that can technically be used as a gpio.
    // The comment matches the name printed on the silkscreen of the board.

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); // D3

    // prevents boot / causes crash:
    //PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1); // TX

    // used for debug output over uart:
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); // D4

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3); // RX

    // prevents boot / causes crash:
    //PIN_FUNC_SELECT(FUNC_U0TXD_BK, FUNC_GPIO4); // D2

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5); // D1

    // prevents boot / causes crash:
    //PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9); // SC

    // prevents boot / causes crash:
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10); // SO

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); // D6

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); // D7

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14); // D5

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15); // D8
}