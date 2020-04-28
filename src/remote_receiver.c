#include <stdio.h>
#include <osapi.h>
#include <c_types.h>
#include <stdlib.h>

#include "gpio_util.h"
#include "pins.h"
#include "i2c_slave.h"
#include "ring_buffer.h"
#include "uart.h"
#include "commands.h"

#define PRINTER_MAX_FEEDRATE 5000

void remote_receiver_init() {

}

void read_uart_input() {
    // example: X:0.00 Y:0.00 Z:0.00 E:0.00 Count ...
    if (ring_buffer_length(&uart_receive_buffer) > 30) { // no response to M114 is ever shorter than 30 chars
        while (uart_receive_buffer.buffer[uart_receive_buffer.start] != 'X') {
            uart_receive_buffer.start++;
        }
        uart_receive_buffer.start++; // consume 'X'
        if (uart_receive_buffer.buffer[uart_receive_buffer.start] != ':') {
            os_printf_plus("read_uart_input 'X' was not followed by ':', got %d 0x%x %c",
                           uart_receive_buffer.buffer[uart_receive_buffer.start],
                           uart_receive_buffer.buffer[uart_receive_buffer.start],
                           uart_receive_buffer.buffer[uart_receive_buffer.start]);
        }
        uart_receive_buffer.start++; // consume ':'
        char x_pos_str[20];
        int i;
        for (i = 0; i < 20 - 1; ++i) {
            uint8 next_byte = uart_receive_buffer.buffer[uart_receive_buffer.start++];
            if ('0' <= next_byte && next_byte <= '9') {
                x_pos_str[i] = (char) next_byte;
            } else if (next_byte == '.') {
                break;
            } else {
                os_printf_plus("read_uart_input number was not followed by '.', got %d 0x%x %c", next_byte, next_byte,
                               next_byte);
                return;
            }
        }
        x_pos_str[i] = '\0';
        int position = atoi(x_pos_str);

        os_printf_plus("read_uart_input number: %d | %s\n", position, x_pos_str);

        ring_buffer_clear(&i2c_slave_send_buffer);
        i2c_slave_send_buffer.buffer[i2c_slave_send_buffer.end++] = 0xaa;
        i2c_slave_send_buffer.buffer[i2c_slave_send_buffer.end++] = position;
        i2c_slave_send_buffer.buffer[i2c_slave_send_buffer.end++] = 0x00;

        ring_buffer_clear(&uart_receive_buffer);
    }
}

uint8 next_byte() {
    uint8 data = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start];
    i2c_slave_receive_buffer.start++;
    i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;
    return data;
}

void remote_receiver_timer() {

    if (ring_buffer_length(&i2c_slave_receive_buffer) >= 4) {
        uint8 byte = next_byte();
        if (byte != 0xFF) {
            os_printf_plus("remote_receiver_timer: expected 0xFF, got 0x%x  %d\n", byte, byte);
            return;
        }

        int8 command = next_byte();

        char printer_message[50];
        printer_message[0] = '\0';


        switch (command) {
            case COMMAND_POSITION: {
                uint8 position_byte = next_byte();
                int position = position_byte * 10;
                os_printf_plus("remote_receiver_timer: pos %d %d \n", position_byte, position);
                os_sprintf(printer_message, "G1 X%d\n", position);
            }
                break;
            case COMMAND_SPEED: {
                uint8 speed_byte = next_byte();
                float speed = ((float) speed_byte / (1 << 7)) * PRINTER_MAX_FEEDRATE;
                os_printf_plus("remote_receiver_timer: speed %d %d.%d \n", speed_byte, (int) speed,
                               ((int) (speed * 100)) % 100);
                os_sprintf(printer_message, "G1 F%d\n", (int) speed);
            }
                break;
            case COMMAND_HOME:
                next_byte();
                os_printf_plus("remote_receiver_timer: home\n");
                os_sprintf(printer_message, "G28 X;\n");
                break;
            case COMMAND_STATUS:
                next_byte();
                os_printf_plus("remote_receiver_timer: status\n");
                os_sprintf(printer_message, "M114\n");
                ring_buffer_clear(&uart_receive_buffer);
                break;
            default:
                os_printf_plus("remote_receiver_timer: unknown command: %d\n", command);
                break;
        }

        byte = next_byte();
        if (byte != 0x00) {
            os_printf_plus("remote_receiver_timer: expected 0x00, got 0x%x  %d\n", byte, byte);
            return;
        }

        ring_buffer_write(&uart_send_buffer, (uint8 *) printer_message);
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end) {
        read_uart_input();
    }
}