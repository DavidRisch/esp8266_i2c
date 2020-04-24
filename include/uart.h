#ifndef ESP8266_I2C_UART_H
#define ESP8266_I2C_UART_H

// baudrate for sending and receiving
#define UART_BAUDRATE 200
#define UART_US_PER_BIT (1000000 / UART_BAUDRATE)

void uart_timer();

void uart_edge();

void my_uart_init();

#endif //ESP8266_I2C_UART_H
