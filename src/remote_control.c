#include <osapi.h>
#include <user_interface.h>
#include "gpio_util.h"
#include "pins.h"
#include "i2c_master.h"
#include "ring_buffer.h"
#include "commands.h"

#define INTERVAL_BUTTON 200 // in ms
#define INTERVAL_STATUS 300 // in ms
#define INTERVAL_SPEED 350 // in ms

static int target_position = 100;
static int last_sent_position = 0;
static int received_position = -1;
static bool request_home = false;

uint32 last_status_command = 0;
uint32 last_speed_command = 0;
bool read_this_cycle = false;

uint32 button_last_times[2];

void set_ready_led() {
    pin_set_value(PIN_REMOTE_CONTROL_LED_READY, target_position == received_position);
}

void set_error_led(bool state) {
    pin_set_value(PIN_REMOTE_CONTROL_LED_ERROR, state);
}

// send message to slave
static void ICACHE_FLASH_ATTR send_message() {

    // sending home command
    if (request_home) {
        os_printf("remote_control COMMAND_HOME\n");
        request_home = !request_home;
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_HOME);
        i2c_master_write_byte(0x00);
        i2c_master_write_byte(0x00);
        return;
    }

    // sending command to move towards target position
    if (target_position != last_sent_position) {
        os_printf("remote_control COMMAND_POSITION: %d\n", target_position);
        last_sent_position = target_position;
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_POSITION);
        i2c_master_write_byte(target_position);
        i2c_master_write_byte(0x00);
        return;
    }

    uint32 time = system_get_time();

    // sending speed
    if ((time - last_speed_command) > INTERVAL_SPEED * 1000) {
        float speed = pin_read_analog();
        os_printf("remote_control COMMAND_SPEED: %d.%d\n", (int) (speed / 1),
                  ((int) (speed * 100)) % 100);
        last_speed_command = time;
        int speed_byte = speed * (1 << 7);
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_SPEED);
        i2c_master_write_byte(speed_byte);
        i2c_master_write_byte(0x00);
    }

    // requesting status
    if ((time - last_status_command) > INTERVAL_STATUS * 1000) {
        os_printf("remote_control COMMAND_STATUS\n");
        last_status_command = time;
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_STATUS);
        i2c_master_write_byte(0x00);
        i2c_master_write_byte(0x00);
        read_this_cycle = false;
    }

    // reading status
    if (!read_this_cycle && (time - last_status_command) > INTERVAL_STATUS / 2 * 1000) {
        os_printf("remote_control READ\n");
        i2c_master_read(3);
        read_this_cycle = true;
    }
}


// read message from slave
static void ICACHE_FLASH_ATTR read_message() {
    if (ring_buffer_length(&i2c_master_receive_buffer) >= 3) {

        uint8 byte = ring_buffer_read_one_byte(&i2c_master_receive_buffer);
        if (byte != 0xFE) {
            os_printf_plus("read_message: expected 0xFE, got 0x%x  %d\n", byte, byte);
            return;
        }

        received_position = ring_buffer_read_one_byte(&i2c_master_receive_buffer);

        os_printf_plus("read_message: received_position %d\n", received_position);

        byte = ring_buffer_read_one_byte(&i2c_master_receive_buffer);
        if (byte != 0xFD) {
            os_printf_plus("read_message: expected 0xFD, got 0x%x  %d\n", byte, byte);
            return;
        }

        set_ready_led();
    }
}

void ICACHE_FLASH_ATTR remote_control_init() {
    os_printf_plus("remote_control_init\n");

    // setting buttons as input
    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_LEFT);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_LEFT]);
    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_RIGHT);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_RIGHT]);
    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_HOME);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_HOME]);

    // setting leds as output
    pin_set_output(PIN_REMOTE_CONTROL_LED_READY);
    pin_set_value(PIN_REMOTE_CONTROL_LED_READY, 1);
    pin_set_output(PIN_REMOTE_CONTROL_LED_ERROR);
    pin_set_value(PIN_REMOTE_CONTROL_LED_ERROR, 1);

    uint32 time = system_get_time();
    int i;
    for (i = 0; i < 2; i++) {
        button_last_times[i] = time;
    }

    set_ready_led();
    set_error_led(false);
}

void remote_control_timer() {
    if (i2c_master_send_buffer.start == i2c_master_send_buffer.end) {
        send_message();
    }

    if (i2c_master_receive_buffer.start != i2c_master_receive_buffer.end) {
        read_message();
    }
}

// handle button interrupts
void remote_control_handle_interrupt(uint32 gpio_status) {
    uint32 time = system_get_time();
    int button = -1;

    if (gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_LEFT)) {
        button = 0;
    } else if (gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_RIGHT)) {
        button = 1;
    } else if (gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_HOME)) {
        button = 2;
    }

    if (button != -1) {
        uint32 delta_time = time - button_last_times[button];
        // os_printf("delta_time: %d\n", delta_time);
        button_last_times[button] = time;

        if (delta_time > INTERVAL_BUTTON * 1000) {

            if (button == 0) {
                if (target_position > COMMAND_POSITION_MIN) {
                    target_position -= 10;
                    set_error_led(false);
                } else {
                    set_error_led(true);
                }
            } else if (button == 1) {
                if (target_position < COMMAND_POSITION_MAX) {
                    target_position += 10;
                    set_error_led(false);
                } else {
                    set_error_led(true);
                }
            } else if (button == 2) {
                request_home = true;
                target_position = COMMAND_POSITION_MAX;
                set_error_led(false);
            }

            set_ready_led();

            os_printf_plus("remote_control_handle_interrupt: %d  pos%d\n", button, target_position);
        }
    }
}
