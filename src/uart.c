#include <ets_sys.h>
#include <osapi.h>

// workaround for problem in hw_timer.c, FRC1 is not used anyway
#undef ETS_FRC1_INTR_ENABLE
#define ETS_FRC1_INTR_ENABLE()

#include <hw_timer.c>

#include "gpio_util.h"
#include "pins.h"
#include "ring_buffer.h"

// baudrate for sending and receiving
#define UART_BAUDRATE 500

// number of samples for each bit
#define SENSE_FACTOR 5

#define COUNTER_WAITING -999

// every point (receive_counter%SENSE_FACTOR)==0 is the edge between two bits
int receive_counter = COUNTER_WAITING;

// cycles since the end of the last frame
int receive_since_last_frame = 0;

// used to improve stability against single flipped bits
bool receive_last_value = 1;

// value of the current word
char receive_binary_word = 0;

ring_buffer_t receive_buffer = {.start=0, .end=0};
ring_buffer_t send_buffer = {.start=0, .end=0};

// send one bit when (send_counter%SENSE_FACTOR)==0
int send_counter = -4 * 10 * SENSE_FACTOR; // ensure 4 frames of constant high for synchronisation

char output_str[RING_BUFFER_LENGTH + 1]; // space for complete ring buffer and one null byte

void timer_function(void *arg) {
    if (send_counter < 0) {
        // before first frame, shortly after boot
        send_counter++;
    } else if (send_counter > 0) {
        if (send_counter % SENSE_FACTOR == 0) {
            int position = send_counter / SENSE_FACTOR;

            if (position >= 1 && position <= 8) {
                // data bit
                bool value = (send_buffer.buffer[send_buffer.start] & (1 << (position - 1))) > 0;
                pin_set_value(PIN_UART_OUT, value);
            } else if (position == 9) {
                // stop bit
                pin_set_value(PIN_UART_OUT, 1);
            } else if (position >= 20) {
                // end of frame
                send_buffer.start++;
                receive_buffer.start %= RING_BUFFER_LENGTH;
                send_counter = -1;
            }
        }
        send_counter++;
    } else if (send_counter == 0 && send_buffer.end != send_buffer.start) {
        // start bit
        pin_set_value(PIN_UART_OUT, 0);
        send_counter = 1;
    }


    bool current_value = pin_read_value(PIN_UART_IN);

    if (receive_counter == COUNTER_WAITING && (!current_value && !receive_last_value)) { // detect start bit (low)


        receive_binary_word = 0x00;
        receive_counter = 0;
        receive_since_last_frame = 0;
    } else if (receive_counter == COUNTER_WAITING) {
        receive_since_last_frame++;
    }

    // check for the center of a bit
    if (receive_counter >= 0 && receive_counter % SENSE_FACTOR == SENSE_FACTOR / 2) {
        int position = receive_counter / SENSE_FACTOR;
        // position 0: start bit (low)
        // position 1-8: data bit (low/high)
        // position 9: stop bit (high)

        if (current_value && position >= 1 && position <= 8) {
            // data bit
            receive_binary_word |= (1 << (position - 1)); // input is LSB(1) to MSB(8)
        } else if (position == 9) {
            // stop bit
            receive_buffer.buffer[receive_buffer.end++] = receive_binary_word;
            receive_buffer.end %= RING_BUFFER_LENGTH;
            receive_counter = COUNTER_WAITING;

            if (receive_binary_word == '\n') {
                ring_buffer_read_line(&receive_buffer, output_str);
                if (output_str[0] != '\0') {
                    os_printf("uart_received: %s\n", output_str);
                }
            }
        }
    }

    receive_last_value = current_value;
    if (receive_counter != COUNTER_WAITING) {
        receive_counter++;
    }
}


void my_uart_init() {
    // try to get SENSE_FACTOR number of timer calls for each bit
    int timer_interval = 1000000 / UART_BAUDRATE / SENSE_FACTOR;
    os_printf("my_uart_init, interval for timer: %dus\n", timer_interval);

    ring_buffer_write(&send_buffer, "M114\nM105\nM105\nM105\n");

    pin_set_input(PIN_UART_IN);
    pin_set_output(PIN_UART_OUT);
    pin_set_value(PIN_UART_OUT, 1);

    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_arm(timer_interval);
    hw_timer_set_func((void (*)(void)) timer_function);
}


