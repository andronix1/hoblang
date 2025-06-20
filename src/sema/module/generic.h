#pragma once

#include "core/mempool.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"

typedef enum {
    SEMA_GENERIC_TYPE
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;

    SemaModule *module;
    SemaType **params;

    union {
        SemaType *type;
    };
} SemaGeneric;

static inline void sema_generic_fill_type(SemaGeneric *generic, SemaType *type) {
    generic->type = type;
}

SemaGeneric *sema_generic_new(Mempool *mempool, SemaGenericKind kind, SemaModule *module, SemaType **params);
SemaValue *sema_generate(SemaGeneric *generic, SemaType **input);
