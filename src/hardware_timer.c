#include "hardware_timer.h"

#include <c_types.h>

// workaround, the function exists in the compiled library but is missing from the header
void ets_isr_unmask(uint32 unmask);

#include <hw_timer.c>

#include "uart.h"
#include "i2c_master.h"

void hardware_timer_interrupt(void *arg) {
    uart_timer();
    i2c_master_timer();
}

void hardware_timer_init() {
    os_printf("hardware_timer_init, interval for timer: %dus\n", UART_US_PER_BIT);
    hw_timer_init(FRC1_SOURCE, 1); // cannot use NMI_SOURCE, causes crash if combined with gpio interrupt
    hw_timer_arm(UART_US_PER_BIT);
    hw_timer_set_func((void (*)(void)) hardware_timer_interrupt);
}

void noop(void *arg) {
    // Do nothing
}

void hardware_timer_stop() {
    hw_timer_set_func((void (*)(void)) noop);
}

