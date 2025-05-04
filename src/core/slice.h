#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char *value;
    size_t length;
} Slice;

Slice slice_from_cstr(const char *value);
bool slice_eq(const Slice *a, const Slice *b);
