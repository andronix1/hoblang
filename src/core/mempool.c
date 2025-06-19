#include "core/mempool.h"
#include "core/arena.h"
#include "core/opt_slice.h"
#include "core/vec.h"
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

typedef struct Mempool {
    void **vectors;
    void **keymaps;
    Arena *arena;
} Mempool;

Mempool *mempool_new(size_t cap) {
    Mempool *mempool = malloc(sizeof(Mempool));
    mempool->vectors = vec_new(void*);
    mempool->keymaps = vec_new(void*);
    mempool->arena = arena_new(cap);
    return mempool;
}

void mempool_add_vec(Mempool *mempool, void *vec) {
    void *old_vec = mempool->vectors;
    vec_push(mempool->vectors, vec);
    vec_attach_pos(vec, vec_top(mempool->vectors));
    if (mempool->vectors != old_vec) {
        for (size_t i = 0; i < vec_len(mempool->vectors) - 1; i++) {
            vec_attach_pos(mempool->vectors[i], &mempool->vectors[i]);
        }
    }
}

void mempool_add_keymap(Mempool *mempool, void *keymap) {
    void *old_vec = mempool->keymaps;
    vec_push(mempool->keymaps, keymap);
    vec_attach_pos(keymap, vec_top(mempool->keymaps));
    if (mempool->keymaps != old_vec) {
        for (size_t i = 0 ; i < vec_len(mempool->keymaps) - 1; i++) {
            vec_attach_pos(mempool->keymaps[i], &mempool->keymaps[i]);
        }
    }
}

inline char *mempool_slice_to_cstr(Mempool *mempool, Slice slice) {
    char *result = vec_new_in(mempool, char);
    vec_resize(result, slice.length + 1);
    memcpy(result, slice.value, slice.length);
    result[slice.length] = '\0';
    return result;
}

char *mempool_opt_slice_to_cstr_or(Mempool *mempool, OptSlice opt, char *or) {
    return opt.has_value ? mempool_slice_to_cstr(mempool, opt.slice) : or;
}

void *__mempool_alloc(Mempool *mempool, size_t size, size_t align) {
    return __arena_alloc(mempool->arena, size, align);
}

void mempool_free(Mempool *mempool) {
    arena_free(mempool->arena);
    for (size_t i = 0; i < vec_len(mempool->keymaps); i++) {
        keymap_free(mempool->keymaps[i]);
    }
    vec_free(mempool->keymaps);
    for (size_t i = 0; i < vec_len(mempool->vectors); i++) {
        vec_free(mempool->vectors[i]);
    }
    vec_free(mempool->vectors);
    free(mempool);
}
