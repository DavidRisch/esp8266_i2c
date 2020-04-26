#include <stdio.h>
#include <osapi.h>
#include <c_types.h>

#include "gpio_util.h"
#include "pins.h"
#include "i2c_slave.h"
#include "ring_buffer.h"
#include "uart.h"


static void send_message_to_printer(int position, int speed) {
    char message1[50];
    os_sprintf(message1, "G1 F%d\n", speed);

    char message2[50];
    os_sprintf(message2, "G1 X%d\n", position);

    os_printf("send_message_to_printer: %s \n", message1);
    os_printf("send_message_to_printer: %s \n", message2);

    //ring_buffer_write(&uart_send_buffer, message1);
    //ring_buffer_write(&uart_send_buffer, message2);
    //Example:
    //G1 F1500     Feedrate 1500mm/m
    //G1 X50       Move 50mm to the right of the origin
}

void remote_receiver_init() {

}

void remote_receiver_timer() {

    if (i2c_slave_receive_buffer.end - i2c_slave_receive_buffer.start >=
        4) { //TODO ring buffer length function (with modulo)
        if (i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start] != 0xFF) {
            os_printf_plus("remote_receiver_timer: expected 0xFF, got 0x%x  %d\n",
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start],
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start]);
            return;
        }
        i2c_slave_receive_buffer.start++;
        i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;

        uint8 position_byte = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start++]; //position from 0 to 20
        i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;
        int position = position_byte * 10;
        os_printf_plus("remote_receiver_timer: pos %d %d \n", position_byte, position);

        uint8 speed_byte = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start++]; //speed from 0 to 128
        i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;
        float speed = ((float) speed_byte / (1 << 7)) * 1500;
        os_printf_plus("remote_receiver_timer: speed %d %d.%d \n", speed_byte, (int) speed,
                       ((int) (speed * 100)) % 100);

        if (i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start] != 0x00) {
            os_printf_plus("remote_receiver_timer: expected 0x00, got 0x%x  %d\n",
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start],
                           i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start]);
            return;
        }
        i2c_slave_receive_buffer.start++;
        i2c_slave_receive_buffer.start %= RING_BUFFER_LENGTH;

        send_message_to_printer(position, speed);
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end) {
        //TODO
    }
}