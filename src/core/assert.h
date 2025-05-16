#pragma once

#include <assert.h>

#define MEMFAIL *((char*)NULL) = 0
// #define MEMFAIL

#define UNREACHABLE do { \
    MEMFAIL; \
    assert(0 && "unreachable"); \
} while (0)

#define TODO assert(0 && "TODO")
