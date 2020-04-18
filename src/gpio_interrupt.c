#include "gpio_interrupt.h"

#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>

#include "gpio_util.h"
#include "pins.h"
#include "uart.h"
#include "i2c_master.h"

bool uart_pin_state;

void gpio_interrupt_edge() {
    // clear the interrupt status
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    if (pin_read_value(PIN_UART_IN) != uart_pin_state) {
        uart_pin_state = !uart_pin_state;
        uart_edge();
    }
}

void gpio_interrupt_init() {
    int i;
    for (i = GPIO_ID_PIN0; i < GPIO_LAST_REGISTER_ID; ++i) {
        gpio_pin_intr_state_set(i, GPIO_PIN_INTR_DISABLE);
    }

    // uart
    uart_pin_state = pin_read_value(PIN_UART_IN);
    gpio_pin_intr_state_set(GPIO_ID_PIN(PIN_UART_IN), GPIO_PIN_INTR_ANYEDGE);

    ETS_GPIO_INTR_ATTACH(&gpio_interrupt_edge, 0);

    // workaround, the function exists in the compiled library but is missing from the header
    void ets_isr_unmask(uint32 unmask);
    ETS_GPIO_INTR_ENABLE();
    ETS_INTR_UNLOCK();
}
