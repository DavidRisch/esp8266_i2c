#include <osapi.h>
#include <user_interface.h>
#include "gpio_util.h"
#include "pins.h"
#include "i2c_master.h"
#include "ring_buffer.h"

static int position = 10;
static int last_sent_position = 0;

uint32 button_last_times[2];

static void send_message(int new_position, float new_speed) {
    os_printf("remote_control send_message: p%d s%d.%d\n", new_position, (int) (new_speed / 1),
              ((int) (new_speed * 100)) % 100);

    int speed_byte = new_speed * (1 << 7);

    i2c_master_write_byte(0xFF);
    i2c_master_write_byte(new_position);
    i2c_master_write_byte(speed_byte);
    i2c_master_write_byte(0x00);

}

void ICACHE_FLASH_ATTR remote_control_init() {
    os_printf("remote_control_init\n");

    pin_set_input(PIN_REMOTE_CONTROL_LEFT_BUTTON);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_LEFT_BUTTON]);
    pin_set_input(PIN_REMOTE_CONTROL_RIGHT_BUTTON);
    PIN_PULLUP_EN(io_mux_address[PIN_REMOTE_CONTROL_RIGHT_BUTTON]);

    pin_set_output(PIN_REMOTE_CONTROL_LED_GREEN);
    pin_set_output(PIN_REMOTE_CONTROL_LED_RED);

    uint32 time = system_get_time();
    int i;
    for (i = 0; i < 2; i++) {
        button_last_times[i] = time;
    }
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


void remote_control_handle_interrupt(uint32 gpio_status) {
    bool left = gpio_status & (1 << PIN_REMOTE_CONTROL_LEFT_BUTTON);
    bool right = gpio_status & (1 << PIN_REMOTE_CONTROL_RIGHT_BUTTON);

    uint32 time = system_get_time();
    int button = -1;

    if (left) {
        button = 0;
    } else if (right) {
        button = 1;
    }

    if (button != -1) {
        uint32 delta_time = time - button_last_times[button];
        // os_printf("delta_time: %d\n", delta_time);
        button_last_times[button] = time;

        if (delta_time > 200 * 1000) { // 200ms

            if (button == 0 && position > 0) {
                position--;
            } else if (button == 1 && position < 20) {
                position++;
            }

            os_printf("remote_control_handle_interrupt: l%d r%d  pos%d\n", left, right, position);
        }
    }


}

