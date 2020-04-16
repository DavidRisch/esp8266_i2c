#include "ets_sys.h"
#include "osapi.h"

#undef ETS_FRC1_INTR_ENABLE
#define ETS_FRC1_INTR_ENABLE()

#include "hw_timer.c"

#include "gpio_util.h"
#include "pins.h"

#define UART_BAUDRATE 500
#define SENSE_FACTOR 5
#define PACKET_LENGTH 8

#define COUNTER_WAITING -999

// every point (counter%SENSE_FACTOR)==0 is the edge between two bits
int counter = COUNTER_WAITING;
bool last_value = 1; // used to detect edges

char values[1000];
int raw_counter = COUNTER_WAITING;
char raw_values[SENSE_FACTOR * 20];
char bin_value = 0;
int since_last = 0;

void timer_function(void *arg) {
    //os_printf("Uart timer triggered\r\n");

    bool value = pin_read_value(PIN_UART_IN);
    //os_printf("v%d c%d   ", value, counter);

    if (counter == COUNTER_WAITING && (!value && !last_value)) { // detect start bit (low)

        pin_set_value(PIN_LED, !pin_get_current_value(PIN_LED));
        bin_value = 0x00;
        raw_values[raw_counter] = 0;
        //os_printf("s_last %d  %s\r\n", since_last, raw_values);

        counter = 0;

        raw_values[raw_counter++] = '\0';
        if (since_last > 10)
            os_printf("\r\n---------------------------------\r\n");
        //os_printf("raw: %s   %d\r\n", raw_values, since_last);
        raw_counter = 0;
        since_last = 0;

        //os_printf("---------------------------------\r\n");
    } else if (counter == COUNTER_WAITING) {
        since_last++;
    }

    if (counter >= 0 || (counter == COUNTER_WAITING && since_last < 50)) {
        if (counter > 0 && counter % SENSE_FACTOR == 0) {
            raw_values[raw_counter++] = ' ';
            /*
            if ((counter/SENSE_FACTOR)%10==0) {
                raw_values[raw_counter] = 0;
                os_printf("raw: %s\r\n", raw_values);
                raw_counter = 0;
            }
            */
        }

        raw_values[raw_counter++] = value ? '#' : ':';

    }

    if (counter >= 0 && counter % SENSE_FACTOR == SENSE_FACTOR / 2) {
        int position = counter / SENSE_FACTOR;
        // position 0: start bit (low)
        // position 1-8: data bit (low/high)
        // position 9: stop bit (high)

        values[position] = '0' + value;

        if (value && position >= 1 && position <= 8) {
            bin_value |= (1 << (position - 1)); // input is LSB(1) to MSB(8)
        }

        if (position == 9) {
            values[10] = '\0';
            os_printf("%c", bin_value);
            //os_printf("stop: %d %d  %c  %s \r\n", counter, raw_counter, bin_value, values);
            raw_values[raw_counter++] = '~';
            //os_printf("%s\r\n", raw_values);
            counter = COUNTER_WAITING;
        }
    }

    last_value = value;
    if (counter != COUNTER_WAITING) {
        counter++;
    }
}


void my_uart_init() {
    os_printf("Init start %d\r\n", 1000000 / UART_BAUDRATE / SENSE_FACTOR);

    pin_set_input(PIN_UART_IN);

    pin_set_output(PIN_LED);

    values[10] = '\0';

    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_arm(1000000 / UART_BAUDRATE / SENSE_FACTOR + 1);
    hw_timer_set_func((void (*)(void)) timer_function);

    os_printf("Init end\r\n");
}


