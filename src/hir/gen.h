#pragma once

#include "hir/api/gen.h"
#include "hir/api/type.h"

typedef struct {
    HirGenParamId *ids;
    HirTypeId type;
} HirGenTypeInfo;

static inline HirGenTypeInfo hir_gen_type_info_new(HirGenParamId *ids, HirTypeId type) {
    HirGenTypeInfo info = {
        .ids = ids,
        .type = type,
    };
    return info;
}
