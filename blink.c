// From: https://github.com/pfalcon/esp-open-sdk

#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "osapi.h"

static const int pin = 2;
static os_timer_t some_timer;

void some_timerfunc(void *arg) {
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) {
    gpio_output_set(0, (1 << pin), 0, 0);
  } else {
    gpio_output_set((1 << pin), 0, 0, 0);
  }
}

void ICACHE_FLASH_ATTR user_init() {
  gpio_init();

  // configure UART TXD to be GPIO1, set as output
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
  gpio_output_set(0, 0, (1 << pin), 0);

  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
  os_timer_arm(&some_timer, 1000, 1);
}
