#include <c_types.h>
#include <driver/uart.h>
#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>

#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"
#include "gpio_util.h"
#include "hardware_timer.h"
#include "gpio_interrupt.h"

bool i2c_is_master;

void sdk_init_done_cb(void) {
    os_printf("sdk_init_done_cb\r\n");

    my_uart_init();

    i2c_is_master = true;

    if (i2c_is_master) {
        i2c_master_init();
        i2c_master_set_address(2);
        i2c_master_write("abc");
        i2c_master_read(3);
    }

    hardware_timer_init();
    gpio_interrupt_init();
}

void ICACHE_FLASH_ATTR user_init() {
    system_update_cpu_freq(SYS_CPU_160MHZ);

    system_timer_reinit();

    uart_init(BIT_RATE_115200, BIT_RATE_9600);

    gpio_util_init();

    system_init_done_cb(sdk_init_done_cb);
}
