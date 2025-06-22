#pragma once

#include "ast/api/generic.h"
#include "core/mempool.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/type.h"

typedef enum {
    SEMA_GENERIC_TYPE,
    SEMA_GENERIC_FUNC,
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;

    Slice name;
    SemaModule *module;
    SemaType **params;

    union {
        struct {
            AstGeneric *source;
            SemaType *type;
        } type;

        struct {
            SemaType *type;
            HirGenScopeId scope;
            HirGenFuncId id;
        } func;
    };
} SemaGeneric;

void sema_generic_fill_type(SemaGeneric *generic, SemaType *type);

static inline void sema_generic_fill_func(SemaGeneric *generic, SemaType *type, HirGenFuncId id) {
    generic->func.type = type;
    generic->func.id = id;
}

void sema_generic_add_ext_function(SemaModule *module, SemaGeneric *generic, Slice name, SemaExtDecl decl);

SemaGeneric *sema_generic_new_type(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, AstGeneric *source);
SemaGeneric *sema_generic_new_func(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, HirGenScopeId scope);
SemaValue *sema_generate(SemaGeneric *generic, SemaType **input);
