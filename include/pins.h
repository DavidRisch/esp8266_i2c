#ifndef ESP8266_I2C_PINS_H
#define ESP8266_I2C_PINS_H

#include <c_types.h>

// convert gpio number to corresponding address in io mux
extern uint32_t *io_mux_address[];

#define PIN_LED 4

#define PIN_UART_IN 5
#define PIN_UART_OUT 4

#define I2C_SCL 14
#define I2C_SDA 12

#endif //ESP8266_I2C_PINS_H
