#pragma once

#include "core/mempool.h"
#include "core/opt_slice.h"
#include "hir/api/code/code.h"
#include "hir/api/mut.h"
#include "hir/api/type.h"
#include <stddef.h>

typedef size_t HirFuncId;

typedef size_t HirLocalId;

typedef struct {
    HirMutability mutability;
    HirTypeId type;
} HirFuncLocal;

static inline HirFuncLocal hir_func_local_new(HirTypeId type, HirMutability mutability) {
    HirFuncLocal local = {
        .type = type,
        .mutability = mutability,
    };
    return local;
}

typedef struct {
    HirTypeId type;
    OptSlice global_name;
    HirCode *code;
    HirFuncLocal *locals;
    size_t loops_count;
} HirFuncInfo;

HirFuncInfo hir_func_info_new(Mempool *mempool, OptSlice global_name);

typedef size_t HirLoopId;
