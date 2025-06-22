#include "hir.h"
#include "core/assert.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

HirGenUsageId hir_add_gen_scope_usage(Hir *hir, HirGenScopeId id, HirTypeId *params, HirGenScopeId *from) {
    HirGenScope *scope = &hir->gen_scopes[id];
    assert(vec_len(params) == vec_len(scope->params));
    for (size_t i = 0; i < vec_len(scope->usages); i++) {
        if (!memcmp(params, scope->usages[i].params, sizeof(HirTypeId) * vec_len(params))) {
            return i;
        }
    }
    vec_push(scope->usages, from ? hir_gen_scope_usage_new_from(params, *from) : hir_gen_scope_usage_new(params));
    if (from) {
        vec_push(hir->gen_scopes[*from].linked_scopes, id);
    }
    return vec_len(scope->usages) - 1;
}

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
