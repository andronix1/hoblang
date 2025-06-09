#pragma once

#include "core/slice.h"
#include "ir/api/decl.h"
#include "ir/api/local.h"
#include "ir/api/stmt/code.h"
#include "ir/api/type.h"
#include "ir/mut.h"

typedef struct {
    IrMutability mutability;
    IrTypeId type;
} IrFuncLocal;

static inline IrFuncLocal ir_func_local_new(IrMutability mutability, IrTypeId type) {
    IrFuncLocal arg = {
        .mutability = mutability,
        .type = type,
    };
    return arg;
}

typedef struct {
    IrTypeId type_id;
    IrCode *code;

    bool is_global;
    Slice global_name;
} IrFunc;

static inline IrFunc ir_func_new(IrTypeId type_id) {
    IrFunc func = {
        .type_id = type_id,
        .code = NULL,
        .is_global = false
    };
    return func;
}

static inline IrFunc ir_func_new_global(Slice name, IrTypeId type_id) {
    IrFunc func = {
        .type_id = type_id,
        .code = NULL,
        .is_global = true,
        .global_name = name
    };
    return func;
}

typedef struct {
    IrFunc func;
    IrFuncLocal *locals;
    IrLocalId *args;
    IrDeclId decl_id;
    IrTypeId type_id;
    size_t loops_count;
} IrFuncInfo;
