#pragma once

#include "ast/api/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef struct {
    Slice name;
} AstGenericParam;

typedef struct AstGeneric {
    AstGenericParam *params;
} AstGeneric;

AstGeneric *ast_generics_new(Mempool *mempool, AstGenericParam *params);

static inline AstGenericParam ast_generic_param_new(Slice name) {
    AstGenericParam param = { .name = name };
    return param;
}

typedef struct AstGenericBuilder {
    AstType **params;
} AstGenericBuilder;

AstGenericBuilder *ast_generic_builder_new(Mempool *mempool, AstType **params);
Slice ast_generic_builder_slice(AstGenericBuilder *builder);
