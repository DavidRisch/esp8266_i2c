#include "gpio_interrupt.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>

#include "gpio_util.h"
#include "pins.h"
#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"

bool uart_pin_state;
bool sda_pin_state;
bool scl_pin_state;
extern bool i2c_is_master;

void gpio_interrupt_edge() {
    // clear the interrupt status
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    // uart
    if (pin_read_value(PIN_UART_IN) != uart_pin_state) {
        uart_pin_state = !uart_pin_state;
        uart_edge();
    }

    // callback for master
    if (i2c_is_master) {
        if (pin_read_value(I2C_SDA) != sda_pin_state) {
            sda_pin_state = !sda_pin_state;
        }

        // scl
        if (pin_read_value(I2C_SCL) != scl_pin_state) {
            scl_pin_state = !scl_pin_state;
        }
    }

    // callback for slave
    if (!i2c_is_master) {
        if (pin_read_value(I2C_SDA) != sda_pin_state) {
            sda_pin_state = !sda_pin_state;
        }

        // scl
        if (pin_read_value(I2C_SCL) != scl_pin_state) {
            scl_pin_state = !scl_pin_state;
        }

        i2c_slave_handle(gpio_status);
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
    uart_pin_state = pin_read_value(PIN_UART_IN);
    gpio_pin_intr_state_set(GPIO_ID_PIN(PIN_UART_IN), GPIO_PIN_INTR_ANYEDGE);

    // sda
    sda_pin_state = pin_read_value(I2C_SDA);
    gpio_pin_intr_state_set(GPIO_ID_PIN(I2C_SDA), GPIO_PIN_INTR_ANYEDGE);

    // scl
    scl_pin_state = pin_read_value(I2C_SCL);
    gpio_pin_intr_state_set(GPIO_ID_PIN(I2C_SCL), GPIO_PIN_INTR_ANYEDGE);

    ETS_GPIO_INTR_ATTACH(&gpio_interrupt_edge, 0);

    // workaround, the function exists in the compiled library but is missing from the header
    void ets_isr_unmask(uint32 unmask);
    ETS_GPIO_INTR_ENABLE();
    ETS_INTR_UNLOCK();
}
