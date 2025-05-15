#pragma once

#include <assert.h>

#define UNREACHABLE do { \
    *((char*)NULL) = 0; \
    assert(0 && "unreachable"); \
} while (0)
#define TODO assert(0 && "TODO")
