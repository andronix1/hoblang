#pragma once

#include "core/mempool.h"
#include "hir/api/func.h"
#include "hir/api/gen.h"
#include <stddef.h>

typedef struct {
    HirGenParamId *params;
    HirGenScopeId *scopes;
    HirFuncId *funcs;
    HirTypeId **usages;
} HirGenScope;

HirGenScope hir_gen_scope_new(Mempool *mempool);
