#include "hir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

HirGenUsageId hir_add_gen_scope_usage(Hir *hir, HirGenScopeId id, HirType **params, HirGenScopeId *from) {
    HirGenScope *scope = &hir->gen_scopes[id];
    assert(vec_len(params) == vec_len(scope->params));
    for (size_t i = 0; i < vec_len(scope->usages); i++) {
        if (!memcmp(params, scope->usages[i].params, sizeof(HirType*) * vec_len(params))) {
            return i;
        }
    }
    vec_push(scope->usages, from ? hir_gen_scope_usage_new_from(params, *from) : hir_gen_scope_usage_new(params));
    if (from) {
        vec_push(hir->gen_scopes[*from].linked_scopes, id);
    }
    return vec_len(scope->usages) - 1;
}
