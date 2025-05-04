#pragma once

#include <stddef.h>
#include "core/slice.h"

void *__keymap_new(size_t size);
void *__keymap_get(void *keymap, Slice key);
void *__keymap_try_insert(void **keymap_ptr, Slice key, void *value);
void keymap_free(void *keymap);

#define keymap_new(TYPE) ((typeof(TYPE)*)__keymap_new(sizeof(TYPE)))
#define keymap_get(KEYMAP, KEY) ((typeof(KEYMAP))__keymap_get(KEYMAP, KEY))
#define keymap_insert_ptr(KEYMAP, KEY, VALUE) ({ \
    typeof(KEYMAP) __val = VALUE; \
    typeof(KEYMAP) *__keymap = KEYMAP; \
    (typeof(KEYMAP))__keymap_try_insert((void**)__keymap, KEY, __val); \
})
#define keymap_insert(KEYMAP, KEY, VALUE) ({ \
    typeof((KEYMAP)[0]) __val = VALUE; \
    typeof(KEYMAP) *__keymap = &(KEYMAP); \
    (typeof(KEYMAP))__keymap_try_insert((void**)__keymap, KEY, &__val); \
})
