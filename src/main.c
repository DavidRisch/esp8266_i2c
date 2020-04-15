#include <user_interface.h>
#include <driver/spi.h>
#include "blink.h"

#include "ets_sys.h"

void sdk_init_done_cb(void) {
    os_printf("sdk_init_done_cb\r\n");

    blink_init();
}

void ICACHE_FLASH_ATTR user_init() {
    uart_init(BIT_RATE_115200, BIT_RATE_9600);

    system_init_done_cb(sdk_init_done_cb);
}