#include "keymap.h"

#include <alloca.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "core/vec.h"

typedef struct {
    Slice key;
    char data[];
} KeyMapElement;

void *__keymap_new(size_t size) {
    return __vec_new(offsetof(KeyMapElement, data) + size);
}

void *__keymap_rev_get(void *keymap, Slice key) {
    for (ssize_t i = (ssize_t)vec_len(keymap) - 1; i >= 0; i--) {
        KeyMapElement *element = vec_at(keymap, i);
        if (slice_eq(key, element->key)) {
            return &element->data;
        }
    }
    return NULL;
}

void *__keymap_get(void *keymap, Slice key) {
    for (size_t i = 0; i < vec_len(keymap); i++) {
        KeyMapElement *element = vec_at(keymap, i);
        if (slice_eq(key, element->key)) {
            return &element->data;
        }
    }
    return NULL;
}

void *__keymap_try_insert(void **keymap_ptr, Slice key, void *value) {
    KeyMapElement *keymap = *keymap_ptr;
    void *has_value = __keymap_get(keymap, key);
    if (has_value) {
        return has_value;
    }
    size_t esize = vec_esize(keymap);
    KeyMapElement *element = alloca(esize);
    element->key = key;
    memcpy(element->data, value, esize - offsetof(KeyMapElement, data));
    vec_push_ptr((*(KeyMapElement**)keymap_ptr), element);
    return NULL;
}

void keymap_free(void *keymap) {
    vec_free(keymap);
}
