#include <string.h>
#include <stdlib.h>

#include "gpio_util.h"
#include "pins.h"
#include "i2c_slave.h"
#include "ring_buffer.h"
#include "uart.h"


static void send_message_to_printer(int position, int speed) {
    char message1[50];
    strcat(message1, "G1 F");
    char buffer1 [10];
    itoa(speed,buffer1,10);
    strcat(message1, buffer1);

    char message2[50];
    strcat(message2, "G1 X");
    char buffer2 [10];
    itoa(position,buffer2,10);
    strcat(message1, buffer2);

    ring_buffer_write(&uart_send_buffer, message1);
    ring_buffer_write(&uart_send_buffer, message2);
    //Example:
    //G1 F1500     Feedrate 1500mm/m
    //G1 X50       50mm  rechts vom ursprung (Berreich 0-200mm)
}

void remote_receiver_init() {

}

void remote_receiver_timer() {

    if (i2c_slave_receive_buffer.end - i2c_slave_receive_buffer.start >= 2) {
        char position_char = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start++]; //position from 0 to 20
        int position = position_char * 10;

        char speed_char = i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.start++]; //speed from 0 to 128
        float speed = (speed_char / (1 << 7)) * 1500;

        send_message_to_printer(position, speed);
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end) {
        //TODO
    }
}