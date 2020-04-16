#include "ring_buffer.h"

void ring_buffer_write(ring_buffer_t *ring_buffer, const char *input) {
    int i;
    for (i = 0; input[i] != '\0'; ++i) {
        ring_buffer->buffer[ring_buffer->end++] = input[i];
    }
}

void ring_buffer_read_line(ring_buffer_t *ring_buffer, char *output) {
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