#pragma once

#include "core/slice.h"
#include "hir/api/type.h"
#include <stddef.h>

typedef enum {
    HIR_EXTERN_FUNC,
    HIR_EXTERN_VAR
} HirExternKind;

typedef struct {
    HirExternKind kind;
    HirType *type;
} HirExternInfo;

static inline HirExternInfo hir_extern_info_new(HirExternKind kind, HirType *type) {
    HirExternInfo info = {
        .type = type,
        .kind = kind,
    };
    return info;
}

typedef struct {
    HirExternInfo *info;
    Slice name;
} HirExternRecord;

static inline HirExternRecord hir_extern_record_new(HirExternInfo *info, Slice name) {
    HirExternRecord rec = {
        .info = info,
        .name = name,
    };
    return rec;
}

typedef size_t HirExternId;

