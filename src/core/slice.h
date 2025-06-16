#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char *value;
    size_t length;
} Slice;

typedef struct {
    bool found;
    Slice left, right;
} SplitSlice;

Slice slice_from_cstr(const char *value);
Slice subslice(Slice source, size_t start, size_t end);
Slice subslice_from(Slice slice, size_t start);
SplitSlice slice_lsplit(Slice slice, char pattern);
bool slice_eq(const Slice a, const Slice b);
Slice slice_new(const char *value, size_t length);
Slice slice_union(Slice a, Slice b);
