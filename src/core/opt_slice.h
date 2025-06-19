#pragma once

#include "core/slice.h"
#include <stdbool.h>

typedef struct {
    bool has_value;
    Slice slice;
} OptSlice;

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

static inline Slice opt_slice_unwrap_or(OptSlice opt, Slice slice) {
    return opt.has_value ? opt.slice : slice;
}
