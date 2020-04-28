#include <osapi.h>
#include <user_interface.h>
#include "gpio_util.h"
#include "pins.h"
#include "i2c_master.h"
#include "ring_buffer.h"
#include "commands.h"

#define INTERVAL_BUTTON 200 // in ms

static int target_position = 10;
static int last_sent_position = 0;
static bool request_home = false;

uint32 last_status_command = 0;
uint32 last_speed_command = 0;

uint32 button_last_times[2];

static void ICACHE_FLASH_ATTR send_message() {

    if (request_home) {
        os_printf("remote_control COMMAND_HOME\n");
        request_home = !request_home;
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_HOME);
        i2c_master_write_byte(0x00);
        i2c_master_write_byte(0x00);
        return;
    }

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
/*
    if ((time - last_speed_command) > 10000 * 1000) {
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
    */

    if ((time - last_status_command) > 8000 * 1000) {
        os_printf("remote_control COMMAND_STATUS\n");
        //i2c_master_read(3);
        last_status_command = time;
        i2c_master_write_byte(0xFF);
        i2c_master_write_byte(COMMAND_STATUS);
        i2c_master_write_byte(0x00);
        i2c_master_write_byte(0x00);
    }

}

void ICACHE_FLASH_ATTR remote_control_init() {
    os_printf_plus("remote_control_init\n");

    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_LEFT);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_LEFT]);
    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_RIGHT);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_RIGHT]);
    pin_set_input(PIN_REMOTE_CONTROL_BUTTON_HOME);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_BUTTON_HOME]);

    pin_set_output(PIN_REMOTE_CONTROL_LED_READY);
    pin_set_value(PIN_REMOTE_CONTROL_LED_READY, 1);
    pin_set_output(PIN_REMOTE_CONTROL_LED_ERROR);
    pin_set_value(PIN_REMOTE_CONTROL_LED_ERROR, 1);


    uint32 time = system_get_time();
    int i;
    for (i = 0; i < 2; i++) {
        button_last_times[i] = time;
    }
}

void remote_control_timer() {

    if (i2c_master_send_buffer.start == i2c_master_send_buffer.end) {
        send_message();
    }

    if (i2c_master_receive_buffer.start != i2c_master_receive_buffer.end) {
        //TODO
    }
}


void remote_control_handle_interrupt(uint32 gpio_status) {
    bool left = gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_LEFT);
    bool right = gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_RIGHT);
    bool home = gpio_status & (1 << PIN_REMOTE_CONTROL_BUTTON_HOME);

    uint32 time = system_get_time();
    int button = -1;

    if (left) {
        button = 0;
    } else if (right) {
        button = 1;
    } else if (home) {
        button = 2;
    }

    if (button != -1) {
        uint32 delta_time = time - button_last_times[button];
        // os_printf("delta_time: %d\n", delta_time);
        button_last_times[button] = time;

        if (delta_time > INTERVAL_BUTTON * 1000) {

            if (button == 0 && target_position > COMMAND_POSITION_MIN) {
                target_position--;
            } else if (button == 1 && target_position < COMMAND_POSITION_MAX) {
                target_position++;
            } else if (button == 2) {
                request_home = true;
                target_position = COMMAND_POSITION_MAX;
            }

            os_printf_plus("remote_control_handle_interrupt: l%d r%d h%d  pos%d\n", left, right, home, target_position);
        }
    }


}

