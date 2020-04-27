#include "ring_buffer.h"

// returns number of bytes written
int ring_buffer_write(ring_buffer_t *ring_buffer, const uint8 *input) {
    int i;
    for (i = 0; input[i] != '\0'; ++i) {
        ring_buffer->buffer[ring_buffer->end++] = input[i];
    }
    return i;
}

void ring_buffer_read_line(ring_buffer_t *ring_buffer, uint8 *output) {
    int i = 0;
    while (ring_buffer->start != ring_buffer->end) {
        output[i] = ring_buffer->buffer[ring_buffer->start++];
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
        return RING_BUFFER_LENGTH - ring_buffer->end - ring_buffer->start;
    }
}
