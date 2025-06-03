#pragma once

#include "core/mempool.h"
#include "ir/api/decl.h"
#include "ir/api/local.h"
#include "ir/api/stmt/code.h"
#include "ir/api/type.h"
#include "ir/mut.h"

typedef struct {
    IrMutability mutability;
    IrTypeId type;
} IrFuncArg;

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
    IrFuncArg *args;
    IrTypeId returns;
    IrCode *code;

    bool is_global;
    Slice global_name;
} IrFunc;

static inline IrFuncArg ir_func_arg_new(IrMutability mutability, IrTypeId type) {
    IrFuncArg arg = {
        .mutability = mutability,
        .type = type,
    };
    return arg;
}

static inline IrFunc ir_func_new(IrFuncArg *args, IrTypeId returns) {
    IrFunc func = {
        .args = args,
        .returns = returns,
        .code = NULL,
        .is_global = false
    };
    return func;
}

static inline IrFunc ir_func_new_global(Slice name, IrFuncArg *args, IrTypeId returns) {
    IrFunc func = {
        .args = args,
        .returns = returns,
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
} IrFuncInfo;

IrFuncInfo ir_func_info_new(Mempool *mempool, IrFunc func, IrDeclId id, IrTypeId type_id);
