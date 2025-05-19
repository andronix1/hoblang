#include "generic.h"
#include "core/mempool.h"
#include "sema/module/api/generic.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "ast/generic.h"
#include "core/vec.h"
#include "sema/module/nodes/type.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

void sema_module_generic_setup(SemaModule *module, AstGeneric *generic, SemaGenericScopeHandle *handle) {
    if ((handle->is_global = sema_module_is_global_scope(module))) {
        handle->ss = sema_ss_new(module, NULL);
        handle->old = sema_module_ss_swap(module, &handle->ss);
    }
    sema_module_push_scope(module);

    SemaGenericParam *params = vec_new_in(module->mempool, SemaGenericParam);
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        SemaType *type = sema_type_new_generic(module->mempool);
        sema_module_push_decl(module, generic->params[i].name, sema_decl_new(module, false,
            sema_value_new_type(module->mempool, type)));
        vec_push(params, sema_generic_param_new(generic->params[i].name, type));
    }
    handle->generic = sema_generic_new(module->mempool, NULL, params);
}

void sema_module_generic_clean(SemaModule *module, SemaType *type, SemaGenericScopeHandle *handle) {
    handle->generic->type = type;
    sema_module_pop_scope(module);
    if (handle->is_global) {
        sema_module_ss_swap(module, handle->old);
    }
}

SemaValue *sema_generic_generate(SemaModule *module, SemaGeneric *generic, AstGenericBuilder *builder) {
    SemaType **types = vec_new_in(module->mempool, SemaType*);
    bool skip = false;
    for (size_t i = 0; i < vec_len(builder->params); i++) {
        SemaType *param = sema_module_analyze_type(module, builder->params[i]);
        if (param) {
            vec_push(types, param);
        } else {
            skip = true;
        }
    }
    if (skip) {
        return NULL;
    }
    if (vec_len(generic->params) != vec_len(types)) {
        sema_module_err(module, ast_generic_builder_slice(builder),
            "expected $l arguments, got $l", vec_len(generic->params), vec_len(types));
        return NULL;
    }
    return sema_value_generate(module->mempool, generic, types);
}
