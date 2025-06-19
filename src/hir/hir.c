#include "hir.h"
#include "core/assert.h"
#include <assert.h>

HirType *hir_resolve_simple_type(Hir *hir, HirTypeId id) {
    HirTypeInfo *info = &hir->types[id];
    switch (info->kind) {
        case HIR_TYPE_INFO_SIMPLE:
            return &info->simple;
        case HIR_TYPE_INFO_RECORD:
            assert(info->record.filled);
            return hir_resolve_simple_type(hir, info->record.id);
    }
    UNREACHABLE;
}
