#include <stdio.h>
#include <osapi.h>
#include <c_types.h>

#include "gpio_util.h"
#include "pins.h"
#include "i2c_slave.h"
#include "ring_buffer.h"
#include "uart.h"
#include "commands.h"

#define PRINTER_MAX_FEEDRATE 5000

void remote_receiver_init() {

}

uint8 next_byte(){
    uint8 data = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start];
    i2c_slave_receive_buffer.start++;
    i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;
    return data;
}

void remote_receiver_timer() {

    if (ring_buffer_length(&i2c_slave_receive_buffer) >= 4) {
        if (next_byte() != 0xFF) {
            os_printf_plus("remote_receiver_timer: expected 0xFF, got 0x%x  %d\n",
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start],
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start]);
            return;
        }

        int8 command = next_byte();

        char printer_message[50];


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
                os_printf_plus("remote_receiver_timer: home\n");
                os_sprintf(printer_message, "G28 X;\n");
                break;
            case COMMAND_STATUS:
                os_printf_plus("remote_receiver_timer: status\n");
                os_sprintf(printer_message, "M114;\n");
                break;
            default:
                os_printf_plus("remote_receiver_timer: unknown command: %d\n", command);
                break;
        }


        if (next_byte() != 0x00) {
            os_printf_plus("remote_receiver_timer: expected 0x00, got 0x%x  %d\n",
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start],
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start]);
            return;
        }

        ring_buffer_write(&uart_send_buffer, (uint8*)printer_message);
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end) {
        //TODO
    }
}