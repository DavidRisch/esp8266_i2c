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

// read from printer
void read_uart_input() {
    // example: X:0.00 Y:0.00 Z:0.00 E:0.00 Count X: 0.0 ...
    if (ring_buffer_length(&uart_receive_buffer) > 10) {

        uint8 input_byte = ring_buffer_read_one_byte(&uart_receive_buffer);
        while (input_byte != 'X') {
            input_byte = ring_buffer_read_one_byte(&uart_receive_buffer);
            if (ring_buffer_length(&uart_receive_buffer) < 8) {
                return;
            }
        }
        input_byte = ring_buffer_read_one_byte(&uart_receive_buffer);
        if (input_byte != ':') {
            os_printf_plus("read_uart_input 'X' was not followed by ':', got %d 0x%x %c\n",
                           input_byte, input_byte, input_byte);
        }
        char x_pos_str[20];
        int i;
        for (i = 0; i < 20 - 1; ++i) {
            input_byte = ring_buffer_read_one_byte(&uart_receive_buffer);
            if ('0' <= input_byte && input_byte <= '9') {
                x_pos_str[i] = (char) input_byte;
            } else if (input_byte == '.') {
                break;
            } else {
                if (i == 0 && input_byte == ' ') {
                    // This is reached when reaching 'X: 0.0' (some stepper motor value) which is not relevant
                } else {
                    x_pos_str[i] = '\0';
                    os_printf_plus("read_uart_input number was not followed by '.', got %d 0x%x %c   '%s'\n",
                                   input_byte, input_byte, input_byte, x_pos_str);
                }
                return;
            }
        }
        x_pos_str[i] = '\0';
        int position = atoi(x_pos_str);

        os_printf_plus("read_uart_input number: %d | %s\n", position, x_pos_str);

        // sends received message to master
        ring_buffer_clear(&i2c_slave_send_buffer);
        ring_buffer_write_one_byte(&i2c_slave_send_buffer, 0xfe);
        ring_buffer_write_one_byte(&i2c_slave_send_buffer, position);
        ring_buffer_write_one_byte(&i2c_slave_send_buffer, 0xfd);
    }
}

void remote_receiver_timer() {
    // check if master sent data
    if (ring_buffer_length(&i2c_slave_receive_buffer) >= 4) {
        uint8 byte = ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
        if (byte != 0xFF) {
            os_printf_plus("remote_receiver_timer: expected 0xFF, got 0x%x  %d\n", byte, byte);
            return;
        }

        int8 command = ring_buffer_read_one_byte(&i2c_slave_receive_buffer);

        char printer_message[50];
        printer_message[0] = '\0';

        // assemble printer message
        switch (command) {
            case COMMAND_POSITION: {
                uint8 position = ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
                os_printf_plus("remote_receiver_timer: pos %d \n", position);
                os_sprintf(printer_message, "G1 X%d\n", position);
            }
                break;
            case COMMAND_SPEED: {
                uint8 speed_byte = ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
                float speed = ((float) speed_byte / (1 << 7)) * PRINTER_MAX_FEEDRATE;
                os_printf_plus("remote_receiver_timer: speed %d %d.%d \n", speed_byte, (int) speed,
                               ((int) (speed * 100)) % 100);
                os_sprintf(printer_message, "G1 F%d\n", (int) speed);
            }
                break;
            case COMMAND_HOME:
                ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
                os_printf_plus("remote_receiver_timer: home\n");
                os_sprintf(printer_message, ";\nM105;\nG28 X;\n");
                break;
            case COMMAND_STATUS:
                ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
                os_printf_plus("remote_receiver_timer: status\n");
                os_sprintf(printer_message, "M114\n");
                break;
            default:
                os_printf_plus("remote_receiver_timer: unknown command: %d\n", command);
                break;
        }

        byte = ring_buffer_read_one_byte(&i2c_slave_receive_buffer);
        if (byte != 0x00) {
            os_printf_plus("remote_receiver_timer: expected 0x00, got 0x%x  %d\n", byte, byte);
            return;
        }

        // send command to printer
        ring_buffer_write(&uart_send_buffer, (uint8 *) printer_message);
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end) {
        read_uart_input();
    }
}
