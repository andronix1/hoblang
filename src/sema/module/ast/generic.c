#include "generic.h"
#include "ast/generic.h"
#include "core/mempool.h"
#include "hir/api/hir.h"
#include "sema/module/api/type.h"
#include "sema/module/decl.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

SemaGeneric *sema_module_generic(SemaModule *module, SemaGenericKind kind, AstGeneric *generic) {
    SemaType **params = vec_new_in(module->mempool, SemaType*);
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        vec_push(params, sema_type_new_generic(module, hir_add_gen_param(module->hir)));
    }
    return sema_generic_new(module->mempool, kind, module, params);
}

SemaGenericCtx sema_module_generic_ctx_setup(SemaModule *module, AstGeneric *generic, SemaGeneric *source) {
    bool is_global = sema_module_is_global_scope(module);
    SemaScopeStack *old_ss = NULL;

    if (is_global) {
        old_ss = sema_module_swap_ss(module, sema_scope_stack_new(module->mempool, 0, NULL));
    }

    sema_module_push_scope(module, NULL);
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        sema_module_push_decl(module, generic->params[i].name, sema_decl_new(module->mempool,
            module, sema_value_new_type(module->mempool, source->params[i])));
    }

    SemaGenericCtx ctx = { .ss = old_ss, .is_global = is_global };
    return ctx;
}

void sema_module_generic_ctx_clean(SemaModule *module, SemaGenericCtx ctx) {
    sema_module_pop_scope(module);
    if (ctx.is_global) {
        sema_module_swap_ss(module, ctx.ss);
    }
}
