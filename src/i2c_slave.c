#include "i2c_slave.h"

#include <gpio.h>
#include <osapi.h>

#include "gpio_interrupt.h"
#include "gpio_util.h"
#include "pins.h"
#include "ring_buffer.h"

// #define I2C_SLAVE_DETAILED_DEBUG
#ifdef I2C_SLAVE_DETAILED_DEBUG
#include <user_interface.h>
#include <uart.h>
#endif

// states for handling with pins
enum state {
    IDLE,
    RECEIVE_ADDRESS,
    RECEIVE_READ_WRITE_BIT,
    DATA,
    WRITE_ACKNOWLEDGE_START,
    WRITE_ACKNOWLEDGE_END,
    CHECK_ACKNOWLEDGE,
    WAIT_FOR_STOP,
    SEND_STOP
};
static enum state i2c_slave_state = IDLE;

// address of slave device
static int i2c_slave_address = 0b0000000;

ring_buffer_t i2c_slave_receive_buffer = {.start = 0, .end = 0};
static int current_byte = 0;
ring_buffer_t i2c_slave_send_buffer = {.start = 0, .end = 0};

static int bit_counter;
static int addressed;
static int write_to_master; // true if sending data to master, false if receiving

unsigned int i2c_edge_last_time = 0;

void i2c_slave_handle_interrupt(uint32 gpio_status, uint32 gpio_values) {

    bool sda_value = gpio_values & (1 << PIN_I2C_SDA);
    bool sda_edge = gpio_status & (1 << PIN_I2C_SDA);
    bool scl_value = gpio_values & (1 << PIN_I2C_SCL);
    bool scl_edge = gpio_status & (1 << PIN_I2C_SCL);

#ifdef I2C_SLAVE_DETAILED_DEBUG
    unsigned int time = system_get_time();
    // number of bits since the last edge
    int bit_number = (time - i2c_edge_last_time + UART_US_PER_BIT / 2) / UART_US_PER_BIT;
    i2c_edge_last_time = time;

    os_printf_plus("--i2c_s_int: SDA: v%d e%d  SCL: v%d e%d  t%d\n",
                   sda_value, sda_edge, scl_value, scl_edge, bit_number);
#endif

    switch (i2c_slave_state) {
        case IDLE:
            // checking if start condition is met
            if (scl_value) {
                bit_counter = 0;
                addressed = 0b0000000;

                // enabling interrupt for clock cycle
                pin_disable_interrupt(PIN_I2C_SDA);
                pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_POSEDGE);
#ifdef I2C_SLAVE_DETAILED_DEBUG
                os_printf_plus("\t\t\t\t\tSTART\n");
#endif

                i2c_slave_state++;
            }
            break;
        case RECEIVE_ADDRESS:

            bit_counter++;
#ifdef I2C_SLAVE_DETAILED_DEBUG
            os_printf_plus("\t\t\t\t\tADDRESS: bit_counter %d  value: %d  addressed: 0x%x %d\n", bit_counter,
                           sda_value,
                           addressed, addressed);
#endif
            if (bit_counter == 7) {
                bit_counter = 0;
                addressed = (addressed | sda_value);
                // comparing received address with own address
                // returning to IDLE state if not equal
                if (i2c_slave_check_address(addressed)) {
                    i2c_slave_state++;
                } else {
                    pin_disable_interrupt(PIN_I2C_SCL);
                    pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
                    i2c_slave_state = IDLE;
                }
            } else {
                addressed = (addressed | sda_value) << 1;
            }

            break;
        case RECEIVE_READ_WRITE_BIT:
            write_to_master = sda_value;
#ifdef I2C_SLAVE_DETAILED_DEBUG
            os_printf_plus("\t\t\t\t\twrite_to_master: %d\n", write_to_master);
#endif

            i2c_slave_state = WRITE_ACKNOWLEDGE_START;
            pin_disable_interrupt(PIN_I2C_SDA);
            pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_NEGEDGE);

            break;
        case DATA:
            if (bit_counter == 0) {
                if (write_to_master) {
                    // if buffer empty
                    if (i2c_slave_send_buffer.start == i2c_slave_send_buffer.end) {
                        // preparing for stop
                        i2c_slave_state = SEND_STOP;
                        pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_POSEDGE);
                    } else {
                        // read next byte from buffer
                        current_byte = i2c_slave_send_buffer.buffer[i2c_slave_send_buffer.start++];
                        os_printf_plus("i2c_slave sending byte: %c  %d\n", current_byte, current_byte);
                    }
                } else {
                    current_byte = 0;
                }
            } else if (bit_counter == 1) {
                if (sda_edge) {
                    // might be stop symbol
                    if (scl_value) {
                        // is stop symbol
#ifdef I2C_SLAVE_DETAILED_DEBUG
                        os_printf_plus("\t\t\t\t\tSTOP\n");
#endif
                        i2c_slave_state = IDLE;
                        // look for next start symbol
                        pin_disable_interrupt(PIN_I2C_SCL);
                        pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
                        return;
                    } else {
                        // not a stop signal, master is just setting the next data bit
                        pin_disable_interrupt(PIN_I2C_SDA); // don't look for stop symbol during data byte
                        return; // and ignore this interrupt
                    };

                }
            }
            bit_counter++;

            if (write_to_master) {
                if (bit_counter == 9) {
                    // falling SCL edge of the last data bit
                    // preparing to check for acknowledge from master
                    bit_counter = 0;
                    pin_i2c_write(PIN_I2C_SDA, 1);

                    i2c_slave_state = CHECK_ACKNOWLEDGE;
                    pin_disable_interrupt(PIN_I2C_SDA);
                    pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_POSEDGE);
                    return;
                }
#ifdef I2C_SLAVE_DETAILED_DEBUG
                os_printf_plus("\t\t\t\t\tWRITE_DATA: bit_counter %d  value: %d  current_byte: 0x%x %d\n", bit_counter,
                               (current_byte >> (8 - bit_counter)) & 1,
                               current_byte, current_byte);
#endif
                // writing current bit of current char/byte
                pin_i2c_write(PIN_I2C_SDA, (current_byte >> (8 - bit_counter)) & 1);
            } else {

                current_byte = current_byte | (sda_value << (8 - bit_counter));
#ifdef I2C_SLAVE_DETAILED_DEBUG
                os_printf_plus("\t\t\t\t\tREAD_DATA: bit_counter %d  value: %d  current_byte: 0x%x %d\n", bit_counter,
                               sda_value,
                               current_byte, current_byte);
#endif
                if (bit_counter == 8) {
#ifdef I2C_SLAVE_DETAILED_DEBUG
                    os_printf_plus("\t\t\t\t\tcurrent_byte: %c 0x%x %d\n", current_byte, current_byte, current_byte);
#endif
                    os_printf_plus("i2c_slave received byte: %c  %d\n", current_byte, current_byte);
                    i2c_slave_receive_buffer.buffer[i2c_slave_receive_buffer.end++] = current_byte;
                    i2c_slave_receive_buffer.end %= RING_BUFFER_LENGTH;

                    bit_counter = 0;

                    i2c_slave_state = WRITE_ACKNOWLEDGE_START;
                    pin_disable_interrupt(PIN_I2C_SDA);
                    pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_NEGEDGE);
                }
            }

            break;
        case WRITE_ACKNOWLEDGE_START:
            // send acknowledge
#ifdef I2C_SLAVE_DETAILED_DEBUG
            os_printf_plus("\t\t\t\t\tWRITE_ACKNOWLEDGE_START\n");
#endif
            pin_i2c_write(PIN_I2C_SDA, 0);

            i2c_slave_state = WRITE_ACKNOWLEDGE_END;

            break;
        case WRITE_ACKNOWLEDGE_END:
            // stop sending acknowledge
#ifdef I2C_SLAVE_DETAILED_DEBUG
            os_printf_plus("\t\t\t\t\tWRITE_ACKNOWLEDGE_END\n");
#endif
            pin_i2c_write(PIN_I2C_SDA, 1);

            i2c_slave_state = DATA;
            if (write_to_master) {
                pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_NEGEDGE); // next data bit
                i2c_slave_handle_interrupt(
                        gpio_status, gpio_values); // the current interrupt must also be used to set the first data bit
            } else {
                pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_POSEDGE); // stop symbol
                pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_POSEDGE); // next data bit
            }

            break;
        case CHECK_ACKNOWLEDGE:
#ifdef I2C_SLAVE_DETAILED_DEBUG
            os_printf_plus("\t\t\t\t\tCHECK_ACKNOWLEDGE\n");
#endif
            if (sda_value) {
                // NACK
                os_printf_plus("i2c_slave received NACK\n");
                i2c_slave_state = WAIT_FOR_STOP;
                pin_disable_interrupt(PIN_I2C_SCL);
                pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_POSEDGE); // stop symbol
            } else {
                // ACK
                os_printf_plus("i2c_slave received ACK\n");
                i2c_slave_state = DATA;
                pin_enable_interrupt(PIN_I2C_SCL, GPIO_PIN_INTR_NEGEDGE); // next data bit
            }

            break;
        case WAIT_FOR_STOP:
            if (sda_edge) {
                // might be stop symbol
                if (scl_value) {
                    // is stop symbol
#ifdef I2C_SLAVE_DETAILED_DEBUG
                    os_printf_plus("\t\t\t\t\tSTOP\n");
#endif
                    i2c_slave_state = IDLE;
                    // look for next start symbol
                    pin_disable_interrupt(PIN_I2C_SCL);
                    pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_NEGEDGE);
                    return;
                }
            }
            break;
        case SEND_STOP:
            pin_i2c_write(PIN_I2C_SDA, 1);
            i2c_slave_state = IDLE;
            // look for next start symbol
            pin_disable_interrupt(PIN_I2C_SCL);
            pin_enable_interrupt(PIN_I2C_SDA, GPIO_PIN_INTR_NEGEDGE);

            break;
    }
#ifdef I2C_SLAVE_DETAILED_DEBUG
    os_printf_plus("++i2c_s_int: SDA: %d   SCL: %d\n\n", pin_read_value(PIN_I2C_SDA), pin_read_value(PIN_I2C_SCL));
#endif
}


// write data to send into send buffer
void i2c_slave_write(const char *data) {
    ring_buffer_write(&i2c_slave_send_buffer, data);
}

// returns true if address is identical to address of slave
bool i2c_slave_check_address(int address) {
#ifdef I2C_SLAVE_DETAILED_DEBUG
    os_printf_plus("\t\t\t\t\ti2c_slave_check_address: %d\n", address);
#endif
    if (address == i2c_slave_address) {
        return true;
    } else {
        return false;
    }
}

// sets address of slave (default: 0000000)
void i2c_slave_set_own_address(int address) {
    i2c_slave_address = address;
}

void i2c_slave_init() {
    os_printf_plus("i2c_slave_init\n");
    pin_i2c_write(PIN_I2C_SCL, 1);
    pin_i2c_write(PIN_I2C_SDA, 1);
}
