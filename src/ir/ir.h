#pragma once

#include "core/mempool.h"
#include "ir/extern.h"
#include "ir/func.h"
#include "ir/type/info.h"
#include "ir/decls.h"
#include "ir/var.h"

typedef struct Ir {
    Mempool *mempool;
    IrTypeInfo *types;
    IrDecl *decls;

    IrFuncInfo *funcs;
    IrVarInfo *vars;
    IrExternInfo *externs;
} Ir;

IrTypeId ir_type_record_resolve_simple(const Ir *ir, IrTypeId id);
