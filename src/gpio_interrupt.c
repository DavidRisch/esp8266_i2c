#include "gpio_interrupt.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>

#include "pins.h"
#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"

extern bool i2c_is_master;

void gpio_interrupt_edge() {
    // clear the interrupt status
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    // uart
    if (gpio_status & (1 << PIN_UART_IN)) {
        uart_edge();
    }

    // i2c
    if (gpio_status & (1 << I2C_SDA) || gpio_status & (1 << I2C_SCL)) {
        if (i2c_is_master) {
            // callback for master
        } else {
            i2c_slave_handle(gpio_status);
        }
    }
}

void pin_enable_interrupt(int pin, GPIO_INT_TYPE state) {
    gpio_pin_intr_state_set(GPIO_ID_PIN(pin), state);
}

void pin_disable_interrupt(int pin) {
    gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_DISABLE);
}

void gpio_interrupt_init() {
    int i;
    for (i = GPIO_ID_PIN0; i < GPIO_LAST_REGISTER_ID; ++i) {
        gpio_pin_intr_state_set(i, GPIO_PIN_INTR_DISABLE);
    }

    // uart
    gpio_pin_intr_state_set(GPIO_ID_PIN(PIN_UART_IN), GPIO_PIN_INTR_ANYEDGE);

    // i2c
    gpio_pin_intr_state_set(GPIO_ID_PIN(I2C_SDA), GPIO_PIN_INTR_ANYEDGE);
    gpio_pin_intr_state_set(GPIO_ID_PIN(I2C_SCL), GPIO_PIN_INTR_ANYEDGE);

    ETS_GPIO_INTR_ATTACH(&gpio_interrupt_edge, 0);

    // workaround, the function exists in the compiled library but is missing from the header
    void ets_isr_unmask(uint32 unmask);
    ETS_GPIO_INTR_ENABLE();
    ETS_INTR_UNLOCK();
}
