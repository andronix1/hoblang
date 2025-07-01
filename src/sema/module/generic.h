#pragma once

#include "ast/api/generic.h"
#include "core/mempool.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/type/type.h"

typedef enum {
    SEMA_GENERIC_TYPE,
    SEMA_GENERIC_FUNC,
    SEMA_GENERIC_GENERIC,
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;

    Slice name;
    SemaModule *module;
    SemaType **additional_params;
    SemaType **gen_params;

    union {
        SemaType *type;

        SemaGeneric *generic;

        struct {
            SemaType *type;
            HirGenScopeId scope;
            HirGenFuncId id;
        } func;
    };
} SemaGeneric;

size_t sema_generic_input_count(SemaGeneric *generic);
SemaType **sema_generic_get_input(SemaGeneric *generic, SemaType **input);
void sema_generic_add_additional_params(SemaGeneric *generic, SemaType **params);

void sema_generic_fill_type(SemaGeneric *generic, SemaType *type);
static inline void sema_generic_fill_func(SemaGeneric *generic, SemaType *type, HirGenFuncId id) {
    generic->func.type = type;
    generic->func.id = id;
}

SemaGeneric *sema_generic_new_type(Mempool *mempool, SemaModule *module, Slice name, SemaType **params);
SemaGeneric *sema_generic_new_generic(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, SemaGeneric *generic);
SemaGeneric *sema_generic_new_func(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, HirGenScopeId scope);
SemaValue *sema_generate(SemaGeneric *generic, SemaType **input);

void sema_generic_type_add_extension(SemaModule *module, SemaGeneric *generic, Slice name, SemaExtDecl decl);
SemaType *sema_generic_type_generate(SemaGeneric *generic, SemaType **inputs);
bool sema_generic_type_search_ext(SemaGeneric *generic, SemaType **inputs, Slice name, SemaExtDecl *output);
