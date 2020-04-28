#ifndef ESP8266_I2C_RING_BUFFER_H
#define ESP8266_I2C_RING_BUFFER_H

#include <c_types.h>

#define RING_BUFFER_LENGTH 1024

typedef struct ring_buffer {
    volatile uint8 buffer[RING_BUFFER_LENGTH];
    volatile int start; // first index in buffer containing none consumed data
    volatile int end; // first index in buffer containing free space
} ring_buffer_t;

void ring_buffer_increment_start(ring_buffer_t *ring_buffer);

void ring_buffer_increment_end(ring_buffer_t *ring_buffer);

uint8 ring_buffer_read_one_byte(ring_buffer_t *ring_buffer);

void ring_buffer_write_one_byte(ring_buffer_t *ring_buffer, const uint8 value);

// returns number of bytes written
int ring_buffer_write(ring_buffer_t *ring_buffer, const uint8 *input);

void ring_buffer_read_line(ring_buffer_t *ring_buffer, uint8 *output);

int ring_buffer_length(ring_buffer_t *ring_buffer);

void ring_buffer_clear(ring_buffer_t *ring_buffer);

#endif //ESP8266_I2C_RING_BUFFER_H
