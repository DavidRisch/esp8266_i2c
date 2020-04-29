#include <osapi.h>
#include "ring_buffer.h"

void ring_buffer_increment_start(ring_buffer_t *ring_buffer) {
    ring_buffer->start++;
    ring_buffer->start %= RING_BUFFER_LENGTH;
}

void ring_buffer_increment_end(ring_buffer_t *ring_buffer) {
    ring_buffer->end++;
    ring_buffer->end %= RING_BUFFER_LENGTH;
}

uint8 ring_buffer_read_one_byte(ring_buffer_t *ring_buffer) {
    uint8 value = ring_buffer->buffer[ring_buffer->start];
    ring_buffer_increment_start(ring_buffer);
    return value;
}

void ring_buffer_write_one_byte(ring_buffer_t *ring_buffer, const uint8 value) {
    ring_buffer->buffer[ring_buffer->end] = value;
    ring_buffer_increment_end(ring_buffer);
}

// returns number of bytes written
int ICACHE_FLASH_ATTR ring_buffer_write(ring_buffer_t *ring_buffer, const uint8 *input) {
    int i;
    for (i = 0; input[i] != '\0'; ++i) {
        ring_buffer->buffer[ring_buffer->end] = input[i];
        ring_buffer_increment_end(ring_buffer);
    }
    return i;
}

void ICACHE_FLASH_ATTR ring_buffer_read_line(ring_buffer_t *ring_buffer, uint8 *output) {
    int i = 0;
    while (ring_buffer->start != ring_buffer->end) {
        output[i] = ring_buffer->buffer[ring_buffer->start];
        ring_buffer_increment_start(ring_buffer);
        if (output[i] == '\n') {
            break;
        } else {
            i++;
        };
    }
    output[i] = '\0';
}

int ring_buffer_length(ring_buffer_t *ring_buffer) {
    if (ring_buffer->end - ring_buffer->start >= 0) {
        return ring_buffer->end - ring_buffer->start;
    } else {
        return RING_BUFFER_LENGTH - ring_buffer->end + ring_buffer->start;
    }
}

void ring_buffer_clear(ring_buffer_t *ring_buffer) {
    ring_buffer->start = 0;
    ring_buffer->end = 0;
}
