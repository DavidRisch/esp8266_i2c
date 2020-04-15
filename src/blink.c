#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "osapi.h"

#include "gpio_util.h"

static const int pin = 4;
static os_timer_t some_timer;

void some_timerfunc(void *arg) {
    os_printf("Timer triggered\r\n");

    pin_set_value(pin, !pin_get_current_value(pin));
}


void blink_init() {

    gpio_init();
    pin_set_output(pin);

    os_timer_setfn(&some_timer, (os_timer_func_t *) some_timerfunc, NULL);
    os_timer_arm(&some_timer, 750, 1);
}
