#include <stdbool.h>
#include <osapi.h>

#include "gpio_util.h"
#include "pins.h"

bool i2c_is_master;
bool remote_is_control;

void ICACHE_FLASH_ATTR role_init() {
    pin_set_input(PIN_ROLE_SELECT);
    PIN_PULLUP_EN(io_mux_address[PIN_ROLE_SELECT]);

    // input on pin "PIN_ROLE_SELECT" determines master and slave
    if (pin_read_value(PIN_ROLE_SELECT)) { // HIGH => master
        os_printf("role_init: A\n");
        i2c_is_master = true;
        remote_is_control = true;
    } else { // LOW => slave
        os_printf("role_init: B\n");
        i2c_is_master = false;
        remote_is_control = false;
    }
}
