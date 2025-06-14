#pragma once

#include <stddef.h>
#include <assert.h>

void *__vec_new(size_t esize);
void *__vec_reserve(void *vec, size_t size);
void *__vec_resize(void *vec, size_t size);
void *__vec_append(void *vec, const void *ptr);
void *__vec_extend(void *vec, void *other);
void *__vec_pop(void *vec);
void *__vec_erase(void *vec);
void *__vec_at(void *vec, size_t idx);
void vec_remove_at(void *vec, size_t idx);
size_t vec_len(const void *vec);
size_t vec_esize(const void *vec);
void vec_attach_pos(void *vec, void **to);
void vec_free(void *vec);

#define vec_new(TYPE) ((TYPE*)__vec_new(sizeof(TYPE)))

#define vec_extend(VEC, OTHER) do { \
    typeof(VEC)* __vec = &(VEC); \
    typeof(VEC)* __other = &(OTHER); \
    *__vec = __vec_extend(*__vec, *__other); \
} while (0)

#define vec_resize(VEC, SIZE) do { \
    typeof(VEC)* __vec = &(VEC); \
    *__vec = __vec_resize(*__vec, SIZE); \
} while (0)

#define vec_reserve(VEC, SIZE) do { \
    typeof(VEC)* __vec = &(VEC); \
    *__vec = __vec_reserve(*__vec, SIZE); \
} while (0)

#define vec_push_ptr(VEC, PTR) do { \
    typeof(VEC)* __vec = &(VEC); \
    typeof(VEC) __ptr = PTR; \
    *__vec = __vec_append(*__vec, __ptr); \
} while(0)

#define vec_push(VEC, VALUE) do { \
    typeof(VEC)* __vec = &(VEC); \
    typeof((VEC)[0]) __val = VALUE; \
    *__vec = __vec_append(*__vec, &__val); \
} while (0)

#define vec_pop(VEC) do { \
    typeof(VEC)* __vec = &(VEC); \
    *__vec = __vec_pop(*__vec); \
} while (0)

#define vec_erase(VEC) do { \
    typeof(VEC)* __vec = &(VEC); \
    *__vec = __vec_erase(*__vec); \
} while (0)

#define vec_at(VEC, IDX) ((typeof(VEC))__vec_at(VEC, IDX))

#define vec_create(FIRST, ...) ({ \
    typeof(FIRST) *__vec2 = vec_new(typeof(FIRST)); \
    typeof(FIRST) __list[] = { FIRST, ##__VA_ARGS__ }; \
    for (size_t __i = 0; __i < sizeof(__list) / sizeof(__list[0]); __i++) { \
        vec_push(__vec2, __list[__i]); \
    }; \
    __vec2; \
})

#define vec_top(VEC) ({ \
    typeof(VEC) *__vec = &(VEC); \
    &((*__vec)[vec_len(*__vec) - 1]); \
})
