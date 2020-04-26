#include "gpio_util.h"
#include "pins.h"
#include "i2c_master.h"
#include "ring_buffer.h"

static char position = 0;
static char last_sent_position = 0;

static void send_message(char new_position, float new_speed) {
    char message[3];
    message[0] = new_position;

    int speed_char = new_speed * (1 << 7);
    message[1] = speed_char;

    message[2] = 0;

    i2c_master_write(message);
}

void remote_control_init() {
    pin_set_input(PIN_REMOTE_CONTROL_LEFT_BUTTON);
    pin_set_input(PIN_REMOTE_CONTROL_RIGHT_BUTTON);
    pin_set_input(PIN_REMOTE_CONTROL_SPEED);

    pin_set_output(PIN_REMOTE_CONTROL_LED_GREEEN);
    pin_set_output(PIN_REMOTE_CONTROL_LED_RED);
}

void remote_control_timer() {

    if (i2c_master_send_buffer.start == i2c_master_send_buffer.end && position != last_sent_position) {
        float speed = pin_read_analog();
        send_message(position, speed);
        last_sent_position = position;
    }

    if (i2c_master_receive_buffer.start != i2c_master_receive_buffer.end) {
        //TODO
    }
}

void remote_control_handle_interrupt() {
    int left_button = pin_read_value(PIN_REMOTE_CONTROL_LEFT_BUTTON);
    int right_button = pin_read_value(PIN_REMOTE_CONTROL_RIGHT_BUTTON);
    position = position + right_button - left_button;
    if (position < 0) {
        position = 0;
    }
    if (position > 20) {
        position = 20;
    }
}

