#ifndef ESP8266_I2C_UART_H
#define ESP8266_I2C_UART_H

#include "ring_buffer.h"

// baudrate for sending and receiving
#define UART_BAUDRATE 250
#define UART_US_PER_BIT (1000000 / UART_BAUDRATE)

ring_buffer_t uart_receive_buffer;
ring_buffer_t uart_send_buffer;

void uart_timer();

void uart_edge();

void my_uart_init();

#endif //ESP8266_I2C_UART_H
