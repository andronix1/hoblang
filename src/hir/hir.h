#pragma once

#include "core/mempool.h"
#include "hir/api/gen.h"
#include "hir/api/gen_scope.h"
#include "hir/decl.h"
#include "hir/func.h"

typedef struct Hir {
    Mempool *mempool;
    HirDeclInfo *decls;
    HirFuncRecord *funcs;
    HirExternInfo *externs_map;
    HirVarInfo *vars;

    HirGenScope *gen_scopes;
    HirGenScopeId *root_gen_scopes;
    size_t gen_params_count;
} Hir;

HirGenUsageId hir_add_gen_scope_usage(Hir *hir, HirGenScopeId id, HirType **params, HirGenScopeId *from);
