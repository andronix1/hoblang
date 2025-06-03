#pragma once

#include "core/mempool.h"
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
        .code = NULL
    };
    return func;
}

typedef struct {
    IrFunc func;
    IrFuncLocal *locals;
    IrLocalId *args;
} IrFuncInfo;

IrFuncInfo ir_func_info_new(Mempool *mempool, IrFunc func);
