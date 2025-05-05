#pragma once

#include <stddef.h>
#include <stdalign.h>

typedef struct Arena Arena;

Arena *arena_new(size_t cap);
void *__arena_alloc(Arena *arena, size_t size, size_t align);
size_t arena_len(Arena *arena);
void arena_free(Arena *arena);

#define arena_alloc(ARENA, TYPE) ((TYPE*)__arena_alloc(ARENA, sizeof(TYPE), alignof(TYPE)))
