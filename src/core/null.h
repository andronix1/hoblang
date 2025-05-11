#pragma once

#include <stdbool.h>

#define NOT_NULL(VALUE) ({ \
    typeof(VALUE) __val = VALUE; \
    if (!__val) return NULL; \
    __val; \
})

#define RET_ON_NULL(VALUE) ({ \
    typeof(VALUE) __val = VALUE; \
    if (!__val) return; \
    __val; \
})

typedef bool (*EqFunc)(const void *a, const void *b);
bool equals_nullable(const void *a, const void *b, EqFunc eq);
