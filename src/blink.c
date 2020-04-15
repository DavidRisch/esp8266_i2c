// From: https://github.com/pfalcon/esp-open-sdk


#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "osapi.h"

static const int pin = 4;
static os_timer_t some_timer;

void some_timerfunc(void *arg) {
    os_printf("Timer triggered\r\n");

    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) & ~((1 << pin)));
    } else {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) | (1 << pin));
    }
}


void blink_init() {

    gpio_init();

    gpio_output_set(0, 0, (1 << pin), 0);

    os_timer_setfn(&some_timer, (os_timer_func_t *) some_timerfunc, NULL);
    os_timer_arm(&some_timer, 750, 1);
}
