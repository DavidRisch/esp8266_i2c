#include <ets_sys.h>
#include <gpio.h>
#include <os_type.h>
#include <osapi.h>

#include "hardware_timer.h"
#include "gpio_util.h"
#include "ring_buffer.h"
#include "pins.h"


enum state {
    IDLE, GO_TO_IDLE, START, STOP, SEND_ADDRESS, SEND_DATA,
    RECEIVE_DATA, SEND_READ_WRITE_BIT, WAIT_FOR_ACKNOWLEDGE, SEND_ACKNOWLEDGE, SEND_NO_ACKNOWLEDGE
};
enum state i2c_master_state = GO_TO_IDLE; //what the master is currently doing
static enum state next_state = IDLE; //what the master is doing next (only used after some states)

ring_buffer_t i2c_master_receive_buffer = {.start=0, .end=0};
static char next_byte_to_send = 0;
static bool resend_byte = false; //set to true when there is no acknowledge from the slave

ring_buffer_t i2c_master_send_buffer = {.start=0, .end=0};
static int receive_counter = 0; //counts how many bytes need to be received
static char current_receiving_byte = 0;

static int address = 0; //address of the slave with which the master is communicating with

static int bit_counter = 0;

static bool wait_one_tick = false; //set to true when the master has to wait for one tick, so that the slave can react

// The timer cycles through 4 steps. Step 0 and 2 change the clock signal.
// Step 1 and 3 are in the middle of the edges of the clock signal. Here the data pin is manipulated.
static int timer_cycle = 0;

void noop(void *arg) {
    // For debugging
}

void i2c_master_timer_function(void *arg) {
    if (timer_cycle == 0) {
        pin_set_value(I2C_SCL, 0);
    } else if (timer_cycle == 1) { //set data pin to send data
        switch (i2c_master_state) {
            case IDLE:
                if (i2c_master_send_buffer.end != i2c_master_send_buffer.start || receive_counter > 0) {
                    i2c_master_state = START;
                }
                break;
            case GO_TO_IDLE:
                pin_set_output(I2C_SDA);
                pin_set_value(I2C_SDA, 1);
                i2c_master_state = IDLE;
                break;
            case SEND_ADDRESS:
                pin_set_value(I2C_SDA, (address & (1 << bit_counter)) > 0);
                bit_counter++;
                if (bit_counter == 7) {
                    i2c_master_state = SEND_READ_WRITE_BIT;
                    bit_counter = 0;
                }
                break;
            case SEND_READ_WRITE_BIT:
                if (i2c_master_send_buffer.end != i2c_master_send_buffer.start) {
                    pin_set_value(I2C_SDA, 0); //send write bit
                    i2c_master_state = WAIT_FOR_ACKNOWLEDGE;
                    next_state = SEND_DATA;
                    wait_one_tick = true;
                } else {
                    pin_set_value(I2C_SDA, 1); //send read bit
                    i2c_master_state = WAIT_FOR_ACKNOWLEDGE;
                    next_state = RECEIVE_DATA;
                    wait_one_tick = true;
                }
                break;
            case WAIT_FOR_ACKNOWLEDGE:
                pin_set_input(I2C_SDA);
                break;
            case SEND_DATA:
                if (bit_counter == 0) {
                    pin_set_output(I2C_SDA);
                    if (!resend_byte) {
                        next_byte_to_send = i2c_master_send_buffer.buffer[i2c_master_send_buffer.start++];
                    }
                    resend_byte = false;
                }
                pin_set_value(I2C_SDA, (next_byte_to_send & (1 << bit_counter)) > 0);
                bit_counter++;
                if (bit_counter == 8) {
                    bit_counter = 0;
                    i2c_master_state = WAIT_FOR_ACKNOWLEDGE;
                    wait_one_tick = true;
                    if (i2c_master_send_buffer.end != i2c_master_send_buffer.start) {
                        next_state = SEND_DATA;
                    } else {
                        next_state = STOP;
                    }
                }
                break;
            case RECEIVE_DATA:
                if (bit_counter == 0) {
                    pin_set_input(I2C_SDA);
                    current_receiving_byte = 0;
                }
                break;
            case SEND_ACKNOWLEDGE:
                pin_set_output(I2C_SDA);
                pin_set_value(I2C_SDA, 1);
                i2c_master_state = next_state;
                wait_one_tick = true;
                break;
            case SEND_NO_ACKNOWLEDGE:
                pin_set_output(I2C_SDA);
                pin_set_value(I2C_SDA, 0);
                i2c_master_state = next_state;
                wait_one_tick = true;
                break;
            case STOP:
                pin_set_output(I2C_SDA);
                pin_set_value(I2C_SDA, 0);
                break;
            default:
                break;
        }
    } else if (timer_cycle == 2) {
        pin_set_value(I2C_SCL, 1);
    } else if (timer_cycle == 3) { //set datapin to create start or stop condition/ read datapin
        if (wait_one_tick) {
            wait_one_tick = false;
        } else {
            switch (i2c_master_state) {
                case START:
                    if (i2c_master_send_buffer.end != i2c_master_send_buffer.start || receive_counter > 0) {
                        pin_set_value(I2C_SDA, 0); //create start condition
                        i2c_master_state = SEND_ADDRESS;
                    }
                    break;
                case WAIT_FOR_ACKNOWLEDGE: {
                    int acknowledge_bit = pin_read_value(I2C_SDA);
                    if (acknowledge_bit) {
                        i2c_master_state = next_state;
                    } else {
                        i2c_master_state = SEND_DATA;
                        resend_byte = true;
                    }
                }
                    break;
                case RECEIVE_DATA: {
                    int received_bit = pin_read_value(I2C_SDA);
                    current_receiving_byte = (current_receiving_byte << 1) | received_bit;
                    bit_counter++;
                    if (bit_counter == 8) {
                        bit_counter = 0;
                        ring_buffer_write(&i2c_master_receive_buffer, &current_receiving_byte);
                        receive_counter--;
                        if (receive_counter > 0) {
                            i2c_master_state = SEND_ACKNOWLEDGE;
                            next_state = RECEIVE_DATA;
                        } else {
                            i2c_master_state = SEND_NO_ACKNOWLEDGE;
                            next_state = STOP;
                        }
                    }
                }
                    break;
                case STOP:
                    pin_set_value(I2C_SDA, 1); //create stop condition
                    i2c_master_state = GO_TO_IDLE;
                    break;
                default:
                    break;
            }
        }
    }
    timer_cycle = (timer_cycle + 1) % 4;


    if (i2c_master_send_buffer.end > 0 && i2c_master_send_buffer.start == i2c_master_send_buffer.end) {
        os_printf_plus("Halting i2c, first buffer sent (for oscilloscope debugging)");
        hw_timer_set_func((void (*)(void)) noop);
    }
}


//creates master interface
void i2c_master_init(int frequency) {
    pin_set_output(I2C_SCL);
    hw_timer_init(NMI_SOURCE, 1);
    hw_timer_arm(1000000 / 2 / frequency);
    hw_timer_set_func((void (*)(void)) i2c_master_timer_function);
}

//reads from slave
void i2c_master_read(int length) {
    receive_counter += length;
}

//writes to slave
void i2c_master_write(const char *data) {
    ring_buffer_write(&i2c_master_send_buffer, data);
}

//sets the address where messages will be send to (Default address: 0000000)
void i2c_master_set_address(int addresscode) {
    address = addresscode;
}

