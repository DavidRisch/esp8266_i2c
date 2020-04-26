#include "uart.h"

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>

#include "gpio_util.h"
#include "pins.h"
#include "ring_buffer.h"

// value of the current word
uint8 uart_receive_binary_word = 0;

ring_buffer_t uart_receive_buffer = {.start=0, .end=0};
ring_buffer_t uart_send_buffer = {.start=0, .end=0};

// send one bit when (uart_send_counter%SENSE_FACTOR)==0
int uart_send_counter = -4 * 10; // ensure 4 frames of constant high for synchronisation

char uart_output_str[RING_BUFFER_LENGTH + 1]; // space for complete ring buffer and one null byte

void uart_timer() {
    if (uart_send_counter < 0) {
        // before first frame, shortly after boot
        uart_send_counter++;
    } else if (uart_send_counter > 0) {
        int position = uart_send_counter;

        if (position >= 1 && position <= 8) {
            // data bit
            bool value = (uart_send_buffer.buffer[uart_send_buffer.start] & (1 << (position - 1))) > 0;
            pin_set_value(PIN_UART_OUT, value);
        } else if (position == 9) {
            // stop bit
            pin_set_value(PIN_UART_OUT, 1);
        } else if (position >= 20) {
            // end of frame
            uart_send_buffer.start++;
            uart_receive_buffer.start %= RING_BUFFER_LENGTH;
            uart_send_counter = -1;

        }
        uart_send_counter++;
    } else if (uart_send_counter == 0 && uart_send_buffer.end != uart_send_buffer.start) {
        // start bit
        pin_set_value(PIN_UART_OUT, 0);
        uart_send_counter = 1;
    }

    if (uart_receive_buffer.start != uart_receive_buffer.end &&
        uart_receive_buffer.buffer[uart_receive_buffer.end - 1] == '\n') {
        ring_buffer_read_line(&uart_receive_buffer, (uint8 *) uart_output_str);
        if (uart_output_str[0] != '\0') {
            os_printf("uart_received: %s\n", uart_output_str);
        }
    }
}

unsigned int edge_last_time = 0;
enum bit_types {
    BIT_START, BIT_0, BIT_1, BIT_2, BIT_3, BIT_4, BIT_5, BIT_6, BIT_7, BIT_STOP
};
int receive_bit_state = BIT_STOP;  //the current state, it will end on the next level change


void uart_edge() {
    bool current_value = pin_read_value(PIN_UART_IN);
    // this function is triggered after an edge, the value before th edge must be the inverse of current_value
    bool previous_value = !current_value;

    unsigned int time = system_get_time();
    // number of bits since the last edge
    int bit_number = (time - edge_last_time + UART_US_PER_BIT / 2) / UART_US_PER_BIT;
    edge_last_time = time;

    if (receive_bit_state >= BIT_START && receive_bit_state <= BIT_7) {
        if (receive_bit_state == BIT_START) { // detect start bit (low)
            uart_receive_binary_word = 0x00;
            receive_bit_state++;
            bit_number--; // start bit is not part of data word
        }
        if (previous_value) {
            int i; // iterate of all bits since the last edge
            for (i = 0; i < bit_number; i++) {
                // input is ordered LSB(BIT_0) to MSB(BIT_7)
                uart_receive_binary_word |= (1 << (receive_bit_state - BIT_0 + i));
            }
        }
        receive_bit_state += bit_number;
        if (receive_bit_state > BIT_7) { // data word is complete
            uart_receive_buffer.buffer[uart_receive_buffer.end++] = uart_receive_binary_word;
            uart_receive_buffer.end %= RING_BUFFER_LENGTH;
            receive_bit_state = BIT_STOP;
            // This relies on detecting the start of BIT_STOP. If the last byte of a transmission were to end with a 1,
            // there wouldn't be a edge at the start of BIT_STOP, the frame would only be saved at the start of the next
            // transmission. This problem does not occur for values <0x80, this includes all ascii characters.
        }
    } else if (receive_bit_state == BIT_STOP && previous_value) {
        receive_bit_state = BIT_START;
    } else {
        os_printf_plus("uart_error\n");
        receive_bit_state = BIT_START;
    }
}

void my_uart_init() {
    pin_set_input(PIN_UART_IN);
    pin_set_output(PIN_UART_OUT);
    pin_set_value(PIN_UART_OUT, 1);

    ring_buffer_write(&uart_send_buffer, (uint8 *) "\nM114\nM105\nM105\nM105\n");
}


