#pragma once

#include "core/mempool.h"
#include "hir/api/func.h"
#include "hir/api/gen.h"
#include <stddef.h>

typedef struct {
    bool is_from;
    HirGenScopeId from;
    HirType **params;
} HirGenScopeUsage;

static inline HirGenScopeUsage hir_gen_scope_usage_new(HirType **params) {
    HirGenScopeUsage usage = {
        .is_from = false,
        .params = params,
    };
    return usage;
}

static inline HirGenScopeUsage hir_gen_scope_usage_new_from(HirType **params, HirGenScopeId from) {
    HirGenScopeUsage usage = {
        .is_from = true,
        .from = from,
        .params = params
    };
    return usage;
}

typedef struct {
    HirGenParamId *params;
    HirGenScopeId *scopes;
    HirGenScopeId *linked_scopes;
    HirFuncId *funcs;
    HirGenScopeUsage *usages;
} HirGenScope;

HirGenScope hir_gen_scope_new(Mempool *mempool);
