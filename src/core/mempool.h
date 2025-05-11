#pragma once

#include <stddef.h>
#include <stdalign.h>
#include "core/vec.h"
#include "core/keymap.h"

typedef struct Mempool Mempool;

Mempool *mempool_new(size_t cap);
void mempool_add_vec(Mempool *mempool, void *vec);
void mempool_add_keymap(Mempool *mempool, void *keymap);
void *__mempool_alloc(Mempool *mempool, size_t size, size_t align);
void mempool_free(Mempool *mempool);

#define vec_new_in(MEMPOOL, TYPE) ({ \
    TYPE *__vec = vec_new(TYPE); \
    mempool_add_vec(MEMPOOL, __vec); \
    __vec; \
})

#define vec_create_in(MEMPOOL, FIRST, ...) ({ \
    typeof(FIRST) *__vec = vec_create(FIRST, ##__VA_ARGS__); \
    mempool_add_vec(MEMPOOL, __vec); \
    __vec; \
})

#define keymap_new_in(MEMPOOL, TYPE) ({ \
    TYPE *__keymap = keymap_new(TYPE); \
    mempool_add_keymap(MEMPOOL, __keymap); \
    __keymap; \
})

#define mempool_alloc(MEMPOOL, TYPE) ((TYPE*)__mempool_alloc(MEMPOOL, sizeof(TYPE), alignof(TYPE)))

#define MEMPOOL_CONSTRUCT(TYPE, FIELDS) { \
    TYPE *out = mempool_alloc(mempool, TYPE); \
    FIELDS; \
    return out; \
}
