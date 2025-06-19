#pragma once

#include "hir/api/extern.h"
#include "hir/api/func.h"
#include "hir/api/var.h"
#include <assert.h>

typedef enum {
    HIR_DECL_FUNC,
    HIR_DECL_EXTERN,
    HIR_DECL_VAR,
} HirDeclInfoKind;

typedef struct {
    HirDeclInfoKind kind;
    bool filled;

    union {
        HirFuncId func;
        HirExternId external;
        HirVarId var;
    };
} HirDeclInfo;

static inline HirDeclInfo hir_decl_info_new() {
    HirDeclInfo info = { .filled = false };
    return info;
}

static inline void hir_decl_info_fill_extern(HirDeclInfo *info, HirExternId id) {
    assert(!info->filled);
    info->kind = HIR_DECL_EXTERN;
    info->external = id;
    info->filled = true;
}

static inline void hir_decl_info_fill_var(HirDeclInfo *info, HirVarId id) {
    assert(!info->filled);
    info->kind = HIR_DECL_VAR;
    info->var = id;
    info->filled = true;
}

static inline void hir_decl_info_fill_func(HirDeclInfo *info, HirFuncId id) {
    assert(!info->filled);
    info->kind = HIR_DECL_FUNC;
    info->func = id;
    info->filled = true;
}

