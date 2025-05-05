#pragma once

#include <stddef.h>
#include <stdalign.h>
#include "core/vec.h"

typedef struct Mempool Mempool;

Mempool *mempool_new(size_t cap);
void mempool_add_vec(Mempool *mempool, void *vec);
void *__mempool_alloc(Mempool *mempool, size_t size, size_t align);
void mempool_free(Mempool *mempool);

#define vec_new_in(MEMPOOL, TYPE) ({ \
    TYPE *__vec = vec_new(TYPE); \
    mempool_add_vec(MEMPOOL, __vec); \
})

#define mempool_alloc(MEMPOOL, TYPE) ((TYPE*)__mempool_alloc(MEMPOOL, sizeof(TYPE), alignof(TYPE)))
