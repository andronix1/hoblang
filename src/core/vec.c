#include "vec.h"
#include <string.h>
#include <malloc.h>

typedef struct {
    size_t esize;
    size_t len;
    size_t cap;
} VecHeader;

void *__vec_new(size_t esize) {
    VecHeader *result = malloc(sizeof(VecHeader));
    result->esize = esize;
    result->len = 0;
    result->cap = 0;
    return &result[1];
}

static inline VecHeader *vec_header(const void *vec) {
    return &((VecHeader*)vec)[-1];
}

inline void *__vec_reserve(void *vec, size_t size) {
    VecHeader *header = vec_header(vec);
    if (header->cap < size) {
        vec = &((VecHeader*)realloc(header, sizeof(VecHeader) + header->esize * size))[1];
        header = vec_header(vec);
        header->cap = size;
    }
    return vec;
}

void *__vec_resize(void *vec, size_t size) {
    vec_reserve(vec, size);
    vec_header(vec)->len = size;
    return vec;
}

inline size_t vec_len(const void *vec) {
    return vec_header(vec)->len;
}

inline size_t vec_esize(const void *vec) {
    return vec_header(vec)->esize;
}

void *__vec_append(void *vec, const void *ptr) {
    vec = __vec_reserve(vec, vec_len(vec) + 1);
    VecHeader *header = vec_header(vec);
    memcpy(&(((char*)vec)[header->len * header->esize]), ptr, header->esize);
    header->len++;
    return vec;
}

void *__vec_pop(void *vec) {
    assert(vec_len(vec) > 0);
    vec_header(vec)->len--;
    return vec;
}

void *__vec_erase(void *vec) {
    vec_header(vec)->len = 0;
    return vec;
}

void *__vec_at(void *vec, size_t idx) {
    VecHeader *header = vec_header(vec);
    assert(idx < header->len);
    return &(((char*)vec)[header->esize * idx]);
}

void vec_free(void *vec) {
    free(vec_header(vec));
}
