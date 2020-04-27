#ifndef ESP8266_I2C_PINS_H
#define ESP8266_I2C_PINS_H

#include <c_types.h>

// convert gpio number to corresponding address in io mux
extern uint32_t *io_mux_address[];

#define PIN_UART_IN 4 // D2
#define PIN_UART_OUT 5 // D1

#define PIN_I2C_SCL 14 // D5
#define PIN_I2C_SDA 12 // D6

#define PIN_ROLE_SELECT 13 // D7

#define PIN_REMOTE_CONTROL_BUTTON_LEFT 4 // D2
#define PIN_REMOTE_CONTROL_BUTTON_RIGHT 5 // D1
#define PIN_REMOTE_CONTROL_BUTTON_HOME 0 // D3
#define PIN_REMOTE_CONTROL_LED_READY 15 // D8
#define PIN_REMOTE_CONTROL_LED_ERROR 13 // D7

#endif //ESP8266_I2C_PINS_H
