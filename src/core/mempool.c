#include "mempool.h"
#include "core/arena.h"
#include "core/vec.h"
#include <stdalign.h>
#include <stdlib.h>

#define DEFAULT_REGION_SIZE 1024

typedef struct Mempool {
    void **vectors;
    Arena *arena;
} Mempool;

Mempool *mempool_new(size_t cap) {
    Mempool *mempool = malloc(sizeof(Mempool));
    mempool->vectors = vec_new(void*);
    mempool->arena = arena_new(cap);
    return mempool;
}

void mempool_add_vec(Mempool *mempool, void *vec) {
    vec_push(mempool->vectors, vec);
}

void *__mempool_alloc(Mempool *mempool, size_t size, size_t align) {
    return __arena_alloc(mempool->arena, size, align);
}

void mempool_free(Mempool *mempool) {
    arena_free(mempool->arena);
    for (size_t i = 0; i < vec_len(mempool->vectors); i++) {
        vec_free(mempool->vectors[i]);
    }
    vec_free(mempool->vectors);
    free(mempool);
}
