#include "slice.h"
#include <assert.h>
#include <string.h>

inline Slice slice_new(const char *value, size_t length) {
    Slice result = {
        .value = value,
        .length = length,
    };
    return result;
}

inline Slice subslice(Slice source, size_t start, size_t end) {
    assert(start <= end);
    source.length = end - start;
    source.value += start;
    return source;
}

inline Slice subslice_from(Slice slice, size_t start) {
    assert(slice.length >= start);
    slice.value = &slice.value[start];
    slice.length -= start;
    return slice;
}

SplitSlice slice_lsplit(Slice slice, char pattern) {
    SplitSlice result = {
        .found = false,
        .left = slice
    };
    size_t idx = 0;
    for (; idx < slice.length; idx++) {
        if (slice.value[idx] == pattern) {
            result.found = true;
            break;
        }
    }
    if (!result.found) {
        return result;
    }
    result.left = subslice(slice, 0, idx);
    result.right = subslice_from(slice, idx + 1);
    return result;
}

Slice slice_from_cstr(const char *value) {
    return slice_new(value, strlen(value));
}

bool slice_eq(const Slice a, const Slice b) {
    if (a.length != b.length) {
        return false;
    }
    return memcmp(a.value, b.value, a.length) == 0;
}

Slice slice_union(Slice a, Slice b) {
    assert(a.value <= b.value);
    a.length = b.value - a.value + b.length;
    return a;
}
