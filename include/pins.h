#ifndef ESP8266_I2C_PINS_H
#define ESP8266_I2C_PINS_H

#include <c_types.h>

// convert gpio number to corresponding address in io mux
extern uint32_t *io_mux_address[];

#define PIN_UART_IN 5 // D1
#define PIN_UART_OUT 4 // D2

#define PIN_I2C_SCL 14 // D5
#define PIN_I2C_SDA 12 // D6

#define PIN_ROLE_SELECT 13 // D7

#endif //ESP8266_I2C_PINS_H
