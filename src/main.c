#include <user_interface.h>
#include <driver/uart.h>
#include <osapi.h>
#include <ets_sys.h>


#include "uart.h"
#include "i2c_master.h"
#include "gpio_util.h"

void sdk_init_done_cb(void) {
    os_printf("sdk_init_done_cb\r\n");

    //my_uart_init();

    i2c_master_init(500);
    i2c_master_set_address(2);
    i2c_master_write("abc");
    i2c_master_read(6);
}

void ICACHE_FLASH_ATTR user_init() {
    system_timer_reinit();

    uart_init(BIT_RATE_115200, BIT_RATE_9600);

    gpio_util_init();

    system_init_done_cb(sdk_init_done_cb);
}