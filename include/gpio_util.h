#ifndef ESP8266_I2C_GPIO_UTIL_H
#define ESP8266_I2C_GPIO_UTIL_H

#include "ets_sys.h"

inline void pin_set_output(int pin) {
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) | (1 << pin));
}

inline void pin_set_input(int pin) {
    GPIO_REG_WRITE(GPIO_ENABLE_ADDRESS, GPIO_REG_READ(GPIO_ENABLE_ADDRESS) & ~(1 << pin));
}

// pin must be output
inline void pin_set_value(int pin, int value) {
    if (value) {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) | (1 << pin));
    } else {
        GPIO_REG_WRITE(GPIO_OUT_ADDRESS, GPIO_REG_READ(GPIO_OUT_ADDRESS) & ~(1 << pin));
    }
}

// pin must be output
inline int pin_get_current_value(int pin) {
    return (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin)) > 0;
}



// pin must be input
inline int pin_read_value(int pin) {
    return (GPIO_REG_READ(GPIO_IN_ADDRESS) & (1 << pin)) > 0;
}

#endif //ESP8266_I2C_GPIO_UTIL_H
