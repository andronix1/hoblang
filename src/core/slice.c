#include "slice.h"
#include <string.h>

Slice slice_from_cstr(const char *value) {
    Slice result = {
        .value = value,
        .length = strlen(value)
    };
    return result;
}

bool slice_eq(const Slice *a, const Slice *b) {
    if (a->length != b->length) {
        return false;
    }
    return memcmp(a->value, b->value, a->length) == 0;
}
