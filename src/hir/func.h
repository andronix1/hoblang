#pragma once

#include "hir/api/func.h"
#include <assert.h>
#include <stdbool.h>

typedef struct {
    bool filled;
    HirLocalId *args;
    HirFuncInfo info;
} HirFuncRecord;

static inline HirFuncRecord hir_func_record_new(HirTypeId type) {
    HirFuncRecord info = {
        .filled = false,
        .info.type = type,
    };
    return info;
}

static inline void hir_func_info_fill(HirFuncRecord *fun, HirFuncInfo info) {
    assert(!fun->filled);
    HirTypeId type = fun->info.type;
    fun->info = info;
    fun->filled = true;
    fun->info.type = type;
}
