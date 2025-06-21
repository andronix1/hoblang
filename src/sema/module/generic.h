#pragma once

#include "core/mempool.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"

typedef enum {
    SEMA_GENERIC_TYPE,
    SEMA_GENERIC_FUNC,
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;

    SemaModule *module;
    SemaType **params;

    union {
        SemaType *type;
        struct {
            SemaType *type;
            HirGenScopeId scope;
            HirGenFuncId id;
        } func;
    };
} SemaGeneric;

static inline void sema_generic_fill_type(SemaGeneric *generic, SemaType *type) {
    generic->type = type;
}

static inline void sema_generic_fill_func(SemaGeneric *generic, SemaType *type, HirGenFuncId id) {
    generic->func.type = type;
    generic->func.id = id;
}

SemaGeneric *sema_generic_new_type(Mempool *mempool, SemaModule *module, SemaType **params);
SemaGeneric *sema_generic_new_func(Mempool *mempool, SemaModule *module, SemaType **params, HirGenScopeId scope);
SemaValue *sema_generate(SemaGeneric *generic, SemaType **input);
