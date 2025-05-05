#include "buffer.h"
#include "core/slice.h"
#include "core/vec.h"

Buffer buffer_new() {
    Buffer buffer = {
        .data = vec_new(char),
        .pos = 0
    };
    return buffer;
}

Slice buffer_value(const Buffer *buffer) {
    return slice_new(buffer->data, buffer->pos);
}

void buffer_erase(Buffer *buffer) {
    buffer->pos = 0;
}

void buffer_putc(Buffer *buffer, char c) {
    if (vec_len(buffer->data) <= buffer->pos + 1) {
        vec_push(buffer->data, c);
    } else {
        buffer->data[buffer->pos] = c;
    }
    buffer->pos++;
}

void buffer_free(Buffer *buffer) {
    vec_free(buffer->data);
}
