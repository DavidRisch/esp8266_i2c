#include "gpio_interrupt.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>

#include "role.h"
#include "pins.h"
#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"

// #define GPIO_INTERRUPT_DETAILED_DEBUG
#ifdef GPIO_INTERRUPT_DETAILED_DEBUG
#include <osapi.h>
#endif

// currently configured states of interrupts for each pin
GPIO_INT_TYPE pin_interrupt_states[GPIO_LAST_REGISTER_ID - GPIO_ID_PIN0 + 1];

void gpio_interrupt_edge() {
    // clear the interrupt status
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    uint32 gpio_values = GPIO_REG_READ(GPIO_IN_ADDRESS);


    int pin;
    for (pin = GPIO_ID_PIN0; pin < GPIO_LAST_REGISTER_ID; ++pin) {
        if (gpio_status & (1u << pin)) {
            bool value = (gpio_values & (1u << pin)) > 0;
#ifdef GPIO_INTERRUPT_DETAILED_DEBUG
            os_printf_plus("interrupt on p%d at v%d\n", pin, value);
#endif

            // check if this interrupt makes sense
            // sometimes an interrupt is triggered on the wrong edge, it needs to be ignored
            if (value && pin_interrupt_states[pin] == GPIO_PIN_INTR_NEGEDGE) {
#ifdef GPIO_INTERRUPT_DETAILED_DEBUG
                os_printf_plus(
                        "interrupt incorrectly triggered on pin on %d at a rising edge (GPIO_PIN_INTR_NEGEDGE)\n", pin);
#endif
                return;
            } else if (!value && pin_interrupt_states[pin] == GPIO_PIN_INTR_POSEDGE) {
#ifdef GPIO_INTERRUPT_DETAILED_DEBUG
                os_printf_plus(
                        "interrupt incorrectly triggered on pin on %d at a falling edge (GPIO_PIN_INTR_POSEDGE)\n",
                        pin);
#endif
                return;
            }
        }
    }


    // uart
    if (gpio_status & (1 << PIN_UART_IN)) {
        uart_edge();
    }

    // i2c
    if (gpio_status & (1 << PIN_I2C_SDA) || gpio_status & (1 << PIN_I2C_SCL)) {
        if (i2c_is_master) {
            // callback for master
        } else {
            i2c_slave_handle_interrupt(gpio_status, gpio_values);
        }
    }
}

void pin_enable_interrupt(int pin, GPIO_INT_TYPE state) {
    pin_interrupt_states[pin] = state;
    gpio_pin_intr_state_set(GPIO_ID_PIN(pin), state);
}

void pin_disable_interrupt(int pin) {
    pin_interrupt_states[pin] = GPIO_PIN_INTR_DISABLE;
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
    if (i2c_is_master) {
        // init for master
    } else {
        gpio_pin_intr_state_set(GPIO_ID_PIN(PIN_I2C_SDA), GPIO_PIN_INTR_NEGEDGE);
    }


    ETS_GPIO_INTR_ATTACH(&gpio_interrupt_edge, 0);

    // workaround, the function exists in the compiled library but is missing from the header
    void ets_isr_unmask(uint32 unmask);
    ETS_GPIO_INTR_ENABLE();
    ETS_INTR_UNLOCK();
}
