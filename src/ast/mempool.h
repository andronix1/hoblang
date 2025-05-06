#pragma once

#include "core/mempool.h"

#define MEMPOOL_CONSTRUCT(TYPE, FIELDS) { \
    TYPE *out = mempool_alloc(mempool, TYPE); \
    FIELDS; \
    return out; \
}
