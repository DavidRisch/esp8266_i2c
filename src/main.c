#include <c_types.h>
#include <driver/uart.h>
#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>

#include "uart.h"
#include "i2c_master.h"
#include "i2c_slave.h"
#include "remote_control.h"
#include "remote_receiver.h"
#include "gpio_util.h"
#include "hardware_timer.h"
#include "gpio_interrupt.h"
#include "role.h"

void sdk_init_done_cb(void) {
    os_printf("sdk_init_done_cb\n");

    role_init();

    my_uart_init();

    if (i2c_is_master) {
        i2c_master_init();
        i2c_master_set_target_address(11);
        //i2c_master_write("abc");
        //i2c_master_read(3);
    } else {
        i2c_slave_init();
        i2c_slave_set_own_address(11);
        //i2c_slave_write("012345");
    }

    if (remote_is_control) {
        remote_control_init();
    } else {
        remote_receiver_init();
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
