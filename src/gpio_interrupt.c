#include "gpio_interrupt.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>

#include "role.h"
#include "pins.h"
#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"
#include "remote_control.h"

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


    // interrupt handling for uart
    if (!remote_is_control && gpio_status & (1 << PIN_UART_IN)) {
        uart_edge();
    }

    // interrupt handling for i2c
    if (gpio_status & (1 << PIN_I2C_SDA) || gpio_status & (1 << PIN_I2C_SCL)) {
        if (!i2c_is_master) {
            i2c_slave_handle_interrupt(gpio_status, gpio_values);
        }
    }

    // interrupt handling for control buttons
    if (remote_is_control &&
        (gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_LEFT) || gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_RIGHT) ||
         gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_HOME))) {
        remote_control_handle_interrupt(gpio_status);
    }
}

// enables interrupt for pin and saves interrupt state
void pin_enable_interrupt(int pin, GPIO_INT_TYPE state) {
    pin_interrupt_states[pin] = state;
    gpio_pin_intr_state_set(GPIO_ID_PIN(pin), state);
}

// disables interrupt for pin
void pin_disable_interrupt(int pin) {
    pin_interrupt_states[pin] = GPIO_PIN_INTR_DISABLE;
    gpio_pin_intr_state_set(GPIO_ID_PIN(pin), GPIO_PIN_INTR_DISABLE);
}

void ICACHE_FLASH_ATTR gpio_interrupt_init() {
    // disabling all interrupts
    int i;
    for (i = GPIO_ID_PIN0; i < GPIO_LAST_REGISTER_ID; ++i) {
        pin_disable_interrupt(i);
    }

    // init for uart
    pin_enable_interrupt(GPIO_ID_PIN(PIN_UART_IN), GPIO_PIN_INTR_ANYEDGE);

    // init for i2c slave
    if (!i2c_is_master) {
        pin_enable_interrupt(GPIO_ID_PIN(PIN_I2C_SDA), GPIO_PIN_INTR_NEGEDGE);
    }

    // init for control buttons
    if (remote_is_control) {
        pin_enable_interrupt(GPIO_ID_PIN(PIN_REMOTE_CONTROL_BUTTON_LEFT), GPIO_PIN_INTR_POSEDGE);
        pin_enable_interrupt(GPIO_ID_PIN(PIN_REMOTE_CONTROL_BUTTON_RIGHT), GPIO_PIN_INTR_POSEDGE);
        pin_enable_interrupt(GPIO_ID_PIN(PIN_REMOTE_CONTROL_BUTTON_HOME), GPIO_PIN_INTR_POSEDGE);
    }


    // multiplexing callback function for interrupts
    ETS_GPIO_INTR_ATTACH(&gpio_interrupt_edge, 0);

    // workaround, the function exists in the compiled library but is missing from the header
    void ets_isr_unmask(uint32 unmask);
    // enable interrupts
    ETS_GPIO_INTR_ENABLE();
    ETS_INTR_UNLOCK();
}
