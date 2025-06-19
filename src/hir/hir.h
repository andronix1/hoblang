#pragma once

#include "core/mempool.h"
#include "hir/api/decl.h"
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
} Hir;

HirTypeId hir_resolve_decl_type_id(Hir *hir, HirDeclId id);
HirType *hir_resolve_simple_type(Hir *hir, HirTypeId id);
