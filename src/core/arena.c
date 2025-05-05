#include "arena.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Arena {
    size_t size, top;
    Arena *prev, *next;
    char content[];
} Arena;

static inline Arena *__arena_new(size_t size, size_t align) {
    size_t total_size = size;
    size_t meta_size = offsetof(Arena, content);

    if (align > meta_size) {
        total_size += align - meta_size;
    } else if (align < meta_size && meta_size % align != 0) {
        total_size += align - meta_size % align;
    }

    size_t alloc_align = align;
    if (alloc_align < alignof(Arena)) {
        alloc_align = alignof(Arena);
    }
    size_t alloc_size = total_size + meta_size;
    if (alloc_size % alloc_align != 0) {
        total_size += alloc_align - alloc_size % alloc_align;
    }
    alloc_size = total_size + meta_size;
    assert(alloc_size % alloc_align == 0);

    Arena *arena = aligned_alloc(alloc_align, alloc_size);
    arena->top = 0;
    arena->size = total_size;
    arena->prev = arena->next = NULL;
    return arena;
}

Arena *arena_new(size_t cap) {
    return __arena_new(cap, 1);
}

static inline void *__arena_take(Arena *arena, size_t size, size_t align) {
    size_t aligned_top = arena->top;
    size_t mem_offset = ((size_t)arena->content + arena->top) % align;
    if (mem_offset != 0) {
        aligned_top += align - mem_offset;
    }

    size_t final_top = aligned_top + size;
    if (final_top > arena->size) {
        return NULL;
    }
    arena->top = final_top;
    return &arena->content[final_top];
}

void *__arena_alloc(Arena *arena, size_t size, size_t align) {
    void *ptr;
    while (!(ptr = __arena_take(arena, size, align))) {
        if (!arena->next) {
            (arena->next = __arena_new(size, align))->prev = arena;
            ptr = __arena_take(arena->next, size, align);
            assert(ptr);
            return ptr;
        }
        arena = arena->next;
    }
    return ptr;
}

void arena_free(Arena *arena) {
    while (arena->next) {
        assert(arena->next->prev == arena);
        arena = arena->next;
    }
    while (arena->prev) {
        assert(arena->prev->next == arena);
        arena = arena->prev;
        free(arena->next);
    }
    free(arena);
}

size_t arena_len(Arena *arena) {
    size_t len = 1;
    while (arena->next) {
        arena = arena->next;
        len++;
    }
    return len;
}
