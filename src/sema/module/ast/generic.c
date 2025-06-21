#include "generic.h"
#include "ast/generic.h"
#include "core/mempool.h"
#include "sema/module/api/type.h"
#include "sema/module/decl.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

SemaGeneric *sema_module_generic_func(SemaModule *module, AstGeneric *generic) {
    SemaType **params = vec_new_in(module->mempool, SemaType*);
    HirGenScopeId gen_scope = hir_add_gen_scope(module->hir);
    if (vec_len(module->gen_scopes) > 0) {
        hir_gen_scope_add_scope(module->hir, *vec_top(module->gen_scopes), gen_scope);
    } else {
        hir_add_root_gen_scope(module->hir, gen_scope);
    }
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        HirGenParamId param = hir_add_gen_param(module->hir);
        hir_gen_scope_add_param(module->hir, gen_scope, param);
        vec_push(params, sema_type_new_gen_param(module, param));
    }
    return sema_generic_new_func(module->mempool, module, params, gen_scope);
}

SemaGeneric *sema_module_generic_type(SemaModule *module, AstGeneric *generic) {
    SemaType **params = vec_new_in(module->mempool, SemaType*);
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        vec_push(params, sema_type_new_generic(module));
    }
    return sema_generic_new_type(module->mempool, module, params);
}

SemaGenericCtx sema_module_generic_ctx_setup(SemaModule *module, AstGeneric *generic, SemaGeneric *source) {
    bool is_global = sema_module_is_global_scope(module);
    SemaScopeStack *old_ss = NULL;

    if (is_global) {
        old_ss = sema_module_swap_ss(module, sema_scope_stack_new(module->mempool, 0, NULL));
    }

    if (source->kind == SEMA_GENERIC_FUNC) {
        vec_push(module->gen_scopes, source->func.scope);
    }

    sema_module_push_scope(module, NULL);
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        sema_module_push_decl(module, generic->params[i].name, sema_decl_new(module->mempool,
            module, sema_value_new_type(module->mempool, source->params[i])));
    }

    SemaGenericCtx ctx = {
        .ss = old_ss,
        .is_global = is_global,
        .is_gen_scope = source->kind == SEMA_GENERIC_FUNC,
    };
    return ctx;
}

void sema_module_generic_ctx_clean(SemaModule *module, SemaGenericCtx ctx) {
    sema_module_pop_scope(module);
    if (ctx.is_gen_scope == SEMA_GENERIC_FUNC) {
        vec_pop(module->gen_scopes);
    }
    if (ctx.is_global) {
        sema_module_swap_ss(module, ctx.ss);
    }
}
