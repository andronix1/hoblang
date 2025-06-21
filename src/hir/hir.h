#pragma once

#include "core/mempool.h"
#include "hir/api/decl.h"
#include "hir/api/gen.h"
#include "hir/api/gen_scope.h"
#include "hir/type.h"
#include "hir/decl.h"
#include "hir/func.h"

typedef struct Hir {
    Mempool *mempool;
    HirTypeInfo *types;
    HirDeclInfo *decls;
    HirFuncRecord *funcs;
    HirExternInfo *externs_map;
    HirVarInfo *vars;

    HirGenScope *gen_scopes;
    HirGenScopeId *root_gen_scopes;
    size_t gen_params_count;
} Hir;

HirGenUsageId hir_add_gen_scope_usage(Hir *hir, HirGenScopeId id, HirTypeId *params);
HirTypeId hir_resolve_decl_type_id(Hir *hir, HirDeclId id);
HirType *hir_resolve_simple_type(Hir *hir, HirTypeId id);
