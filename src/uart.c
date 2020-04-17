#include <ets_sys.h>
#include <osapi.h>

#include "hardware_timer.h"
#include "gpio_util.h"
#include "pins.h"
#include "ring_buffer.h"

// baudrate for sending and receiving
#define UART_BAUDRATE 500

// number of samples for each bit
#define SENSE_FACTOR 5

#define COUNTER_WAITING -999

// every point (uart_receive_counter%SENSE_FACTOR)==0 is the edge between two bits
int uart_receive_counter = COUNTER_WAITING;

// cycles since the end of the last frame
int uart_receive_since_last_frame = 0;

// used to improve stability against single flipped bits
bool uart_receive_last_value = 1;

// value of the current word
char uart_receive_binary_word = 0;

ring_buffer_t uart_receive_buffer = {.start=0, .end=0};
ring_buffer_t uart_send_buffer = {.start=0, .end=0};

// send one bit when (uart_send_counter%SENSE_FACTOR)==0
int uart_send_counter = -4 * 10 * SENSE_FACTOR; // ensure 4 frames of constant high for synchronisation

char uart_output_str[RING_BUFFER_LENGTH + 1]; // space for complete ring buffer and one null byte

void timer_function(void *arg) {
    if (uart_send_counter < 0) {
        // before first frame, shortly after boot
        uart_send_counter++;
    } else if (uart_send_counter > 0) {
        if (uart_send_counter % SENSE_FACTOR == 0) {
            int position = uart_send_counter / SENSE_FACTOR;

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
        }
        uart_send_counter++;
    } else if (uart_send_counter == 0 && uart_send_buffer.end != uart_send_buffer.start) {
        // start bit
        pin_set_value(PIN_UART_OUT, 0);
        uart_send_counter = 1;
    }


    bool current_value = pin_read_value(PIN_UART_IN);

    if (uart_receive_counter == COUNTER_WAITING &&
        (!current_value && !uart_receive_last_value)) { // detect start bit (low)


        uart_receive_binary_word = 0x00;
        uart_receive_counter = 0;
        uart_receive_since_last_frame = 0;
    } else if (uart_receive_counter == COUNTER_WAITING) {
        uart_receive_since_last_frame++;
    }

    // check for the center of a bit
    if (uart_receive_counter >= 0 && uart_receive_counter % SENSE_FACTOR == SENSE_FACTOR / 2) {
        int position = uart_receive_counter / SENSE_FACTOR;
        // position 0: start bit (low)
        // position 1-8: data bit (low/high)
        // position 9: stop bit (high)

        if (current_value && position >= 1 && position <= 8) {
            // data bit
            uart_receive_binary_word |= (1 << (position - 1)); // input is LSB(1) to MSB(8)
        } else if (position == 9) {
            // stop bit
            uart_receive_buffer.buffer[uart_receive_buffer.end++] = uart_receive_binary_word;
            uart_receive_buffer.end %= RING_BUFFER_LENGTH;
            uart_receive_counter = COUNTER_WAITING;

            if (uart_receive_binary_word == '\n') {
                ring_buffer_read_line(&uart_receive_buffer, uart_output_str);
                if (uart_output_str[0] != '\0') {
                    os_printf("uart_received: %s\n", uart_output_str);
                }
            }
        }
    }

    uart_receive_last_value = current_value;
    if (uart_receive_counter != COUNTER_WAITING) {
        uart_receive_counter++;
    }
}


void my_uart_init() {
    // try to get SENSE_FACTOR number of timer calls for each bit
    int timer_interval = 1000000 / UART_BAUDRATE / SENSE_FACTOR;
    os_printf("my_uart_init, interval for timer: %dus\n", timer_interval);

    ring_buffer_write(&uart_send_buffer, "M114\nM105\nM105\nM105\n");

    pin_set_input(PIN_UART_IN);
    pin_set_output(PIN_UART_OUT);
    pin_set_value(PIN_UART_OUT, 1);

    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_arm(timer_interval);
    hw_timer_set_func((void (*)(void)) timer_function);
}


