#include "ets_sys.h"
#include "osapi.h"

#undef ETS_FRC1_INTR_ENABLE
#define ETS_FRC1_INTR_ENABLE()

#include "hw_timer.c"

#include "gpio_util.h"
#include "pins.h"

// baudrate for sending and receiving
#define UART_BAUDRATE 500

// number of samples for each bit
#define SENSE_FACTOR 5

#define COUNTER_WAITING -999

// every point (counter%SENSE_FACTOR)==0 is the edge between two bits
int counter = COUNTER_WAITING;

// cycles since the end of the last word
int since_last_word = 0;

// used to improve stability against single flipped bits
bool last_value = 1;

// value of the current word
char binary_word = 0;


void timer_function(void *arg) {
    bool current_value = pin_read_value(PIN_UART_IN);

    if (counter == COUNTER_WAITING && (!current_value && !last_value)) { // detect start bit (low)
        // check if this is the beginning of a new transmission
        if (since_last_word > 10) {
            os_printf("\r\n---------------------------------\r\n");
        }

        binary_word = 0x00;
        counter = 0;
        since_last_word = 0;
    } else if (counter == COUNTER_WAITING) {
        since_last_word++;
    }

    // check for the center of a bit
    if (counter >= 0 && counter % SENSE_FACTOR == SENSE_FACTOR / 2) {
        int position = counter / SENSE_FACTOR;
        // position 0: start bit (low)
        // position 1-8: data bit (low/high)
        // position 9: stop bit (high)

        if (current_value && position >= 1 && position <= 8) {
            // data bit
            binary_word |= (1 << (position - 1)); // input is LSB(1) to MSB(8)
        } else if (position == 9) {
            // stop bit
            os_printf("%c", binary_word);
            counter = COUNTER_WAITING;
        }
    }

    last_value = current_value;
    if (counter != COUNTER_WAITING) {
        counter++;
    }
}


void my_uart_init() {
    // try to get SENSE_FACTOR number of timer calls for each bit
    int timer_interval = 1000000 / UART_BAUDRATE / SENSE_FACTOR;
    os_printf("my_uart_init, interval for timer: %dus\r\n", timer_interval);

    pin_set_input(PIN_UART_IN);

    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_arm(timer_interval);
    hw_timer_set_func((void (*)(void)) timer_function);
}


