#pragma once

#include "core/opt_slice.h"
#include "hir/api/const.h"
#include "hir/api/type.h"
#include <stddef.h>

typedef size_t HirVarId;

typedef struct {
    OptSlice global_name;
    HirTypeId type;
    bool initialized;
    HirConst initializer;
} HirVarInfo;

static inline HirVarInfo hir_var_info_new(OptSlice global_name, HirTypeId type) {
    HirVarInfo info = {
        .global_name = global_name,
        .type = type,
        .initialized = false
    };
    return info;
}
