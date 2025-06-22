#include "gen_scope.h"

HirGenScope hir_gen_scope_new(Mempool *mempool) {
    HirGenScope scope = {
        .scopes = vec_new_in(mempool, HirGenScopeId),
        .linked_scopes = vec_new_in(mempool, HirGenScopeId),
        .funcs = vec_new_in(mempool, HirFuncId),
        .params = vec_new_in(mempool, HirGenParamId),
        .usages = vec_new_in(mempool, HirGenScopeUsage),
    };
    return scope;
}
