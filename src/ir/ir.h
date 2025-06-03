#pragma once

#include "core/mempool.h"
#include "ir/func.h"
#include "ir/type/info.h"
#include "ir/decls.h"

typedef struct Ir {
    Mempool *mempool;
    IrTypeInfo *types;
    IrDecl *decls;
    IrFuncInfo *funcs;
} Ir;

IrTypeId ir_type_record_resolve_simple(const Ir *ir, IrTypeId id);
