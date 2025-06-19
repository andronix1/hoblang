#pragma once

#include "hir/api/type.h"
#include <assert.h>

typedef enum {
    HIR_TYPE_INFO_SIMPLE,
    HIR_TYPE_INFO_RECORD,
} HirTypeInfoKind;

typedef struct {
    HirTypeInfoKind kind;

    union {
        HirType simple;
        struct {
            bool filled;
            HirTypeId id;
        } record;
    };
} HirTypeInfo;

static inline HirTypeInfo hir_type_info_new_simple(HirType type) {
    HirTypeInfo info = {
        .kind = HIR_TYPE_INFO_SIMPLE,
        .simple = type
    };
    return info;
}

static inline void hir_type_info_record_fill(HirTypeInfo *info, HirTypeId type) {
    assert(!info->record.filled);
    assert(info->kind == HIR_TYPE_INFO_RECORD);
    info->record.id = type;
    info->record.filled = true;
}

static inline HirTypeInfo hir_type_info_new_record() {
    HirTypeInfo info = {
        .kind = HIR_TYPE_INFO_RECORD,
        .record = {
            .filled = false,
        }
    };
    return info;
}

