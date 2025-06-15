#pragma once

#include "core/slice.h"
#include "ir/api/const.h"
#include "ir/api/decl.h"
#include "ir/api/type.h"

typedef struct {
    IrTypeId type;
    IrConst *initializer;

    bool is_global;
    Slice global_name;
} IrVar;

static inline IrVar ir_var_new(IrTypeId type) {
    IrVar var = {
        .type = type,
        .is_global = false,
        .initializer = NULL,
    };
    return var;
}

static inline IrVar ir_var_new_global(IrTypeId type, Slice name) {
    IrVar var = {
        .type = type,
        .is_global = true,
        .global_name = name,
        .initializer = NULL,
    };
    return var;
}

typedef struct {
    IrVar var;
    IrDeclId decl_id;
} IrVarInfo;

static inline IrVarInfo ir_var_info_new(IrVar var, IrDeclId decl_id) {
    IrVarInfo info = {
        .var = var,
        .decl_id = decl_id
    };
    return info;
}

