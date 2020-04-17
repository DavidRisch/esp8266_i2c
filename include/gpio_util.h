#ifndef ESP8266_I2C_GPIO_UTIL_H
#define ESP8266_I2C_GPIO_UTIL_H

#include "ets_sys.h"

void pin_set_output(int pin);

void pin_set_input(int pin);

// pin must be output
void pin_set_value(int pin, int value);

// pin must be output
int pin_get_current_value(int pin);


// pin must be input
int pin_read_value(int pin);

// returns float in range [0, 1)
float pin_read_analog();

#endif //ESP8266_I2C_GPIO_UTIL_H
