#ifndef ESP8266_I2C_HARDWARE_TIMER_H
#define ESP8266_I2C_HARDWARE_TIMER_H

#include <c_types.h>

typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;

void hw_timer_arm(u32 val);

void ICACHE_FLASH_ATTR hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type, u8 req);

void hw_timer_set_func(void (*user_hw_timer_cb_set)(void));

#endif //ESP8266_I2C_HARDWARE_TIMER_H
