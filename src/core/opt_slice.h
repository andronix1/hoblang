#pragma once

#include "core/slice.h"
#include <stdbool.h>

typedef struct {
    bool has_value;
    Slice slice;
} OptSlice;

static inline bool opt_slice_eq(const OptSlice *a, const OptSlice *b) {
    return a->has_value == b->has_value && (!a->has_value || slice_eq(a->slice, b->slice));
}

static inline OptSlice opt_slice_new_null() {
    OptSlice opt = { .has_value = false };
    return opt;
}

static inline OptSlice opt_slice_new_value(Slice value) {
    OptSlice opt = {
        .has_value = true,
        .slice = value,
    };
    return opt;
}

static inline OptSlice opt_slice_map_or(OptSlice opt, Slice slice) {
    return opt_slice_new_value(opt.has_value ? opt.slice : slice);
}

static inline Slice opt_slice_unwrap_or(OptSlice opt, Slice slice) {
    return opt.has_value ? opt.slice : slice;
}
