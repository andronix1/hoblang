#pragma once

#define NOT_NULL(VALUE) ({ \
    typeof(VALUE) __val = VALUE; \
    if (!__val) return NULL; \
    __val; \
})
