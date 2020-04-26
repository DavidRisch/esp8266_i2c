#ifndef ESP8266_I2C_REMOTE_CONTROL_H
#define ESP8266_I2C_REMOTE_CONTROL_H

void remote_control_init();

void remote_control_timer();

void remote_control_handle_interrupt(uint32 gpio_status);

#endif
