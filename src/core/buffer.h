#pragma once

#include "core/slice.h"
#include <stddef.h>

typedef struct {
    char *data;
    size_t pos;
} Buffer;

Buffer buffer_new();
Slice buffer_value(const Buffer *buffer);
void buffer_erase(Buffer *buffer);
void buffer_putc(Buffer *buffer, char c);
void buffer_free(Buffer *buffer);
