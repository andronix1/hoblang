#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char *value;
    size_t length;
} Slice;

Slice slice_from_cstr(const char *value);
Slice subslice(Slice source, size_t start, size_t end);
bool slice_eq(const Slice a, const Slice b);
Slice slice_new(const char *value, size_t length);
Slice slice_union(Slice a, Slice b);
