#ifndef ESP8266_I2C_GPIO_INTERRUPT_H
#define ESP8266_I2C_GPIO_INTERRUPT_H

#include <c_types.h>
#include <gpio.h>

void pin_enable_interrupt(int pin, GPIO_INT_TYPE state);

void pin_disable_interrupt(int pin);

void gpio_interrupt_init();

#endif //ESP8266_I2C_GPIO_INTERRUPT_H
