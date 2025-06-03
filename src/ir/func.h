#pragma once

#include "ir/api/stmt/code.h"
#include "ir/api/type.h"
#include "ir/mut.h"

typedef struct {
    IrMutability mutability;
    IrTypeId type;
} IrFuncArg;

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
