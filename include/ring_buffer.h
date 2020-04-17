#ifndef ESP8266_I2C_RING_BUFFER_H
#define ESP8266_I2C_RING_BUFFER_H

#define RING_BUFFER_LENGTH 1024

typedef struct ring_buffer {
    volatile char buffer[RING_BUFFER_LENGTH];
    volatile int start; // first index in buffer containing none consumed data
    volatile int end; // first index in buffer containing free space
} ring_buffer_t;

// returns number of bytes written
int ring_buffer_write(ring_buffer_t *ring_buffer, const char *input);

void ring_buffer_read_line(ring_buffer_t *ring_buffer, char *output);

#endif //ESP8266_I2C_RING_BUFFER_H
