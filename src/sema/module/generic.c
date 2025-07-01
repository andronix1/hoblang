#include "generic.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type/alias.h"
#include "sema/module/value.h"
#include "sema/module/const.h"
#include <string.h>

void sema_generic_fill_type(SemaGeneric *generic, SemaType *type) {
    assert(!type->alias);
    type->alias = sema_type_alias_new(generic->module->mempool, generic->name);
    generic->type = type;
}

size_t sema_generic_input_count(SemaGeneric *generic) {
    return vec_len(generic->gen_params) - vec_len(generic->additional_params);
}

SemaType **sema_generic_get_input(SemaGeneric *generic, SemaType **input) {
    assert(vec_len(input) == sema_generic_input_count(generic));
    SemaModule *module = generic->module;
    SemaType **output = vec_new_in(module->mempool, SemaType*);
    vec_extend(output, generic->additional_params);
    vec_extend(output, input);
    return output;
}

SemaValue *sema_generate(SemaGeneric *generic, SemaType **input) {
    assert(vec_len(input) == vec_len(generic->gen_params) - vec_len(generic->additional_params));
    input = sema_generic_get_input(generic, input);
    SemaModule *module = generic->module;
    switch (generic->kind) {
        case SEMA_GENERIC_TYPE:
            return sema_value_new_type(module->mempool, sema_type_new_generate(module->mempool, generic, input));
        case SEMA_GENERIC_FUNC:
            return sema_value_new_runtime_const(module->mempool, sema_const_new_gen_func(module->mempool,
                sema_type_replace(module->mempool, generic->func.type, generic->gen_params, input),
                    generic->func.scope, generic->func.id, input));
        case SEMA_GENERIC_GENERIC: {
            SemaGeneric *copy = mempool_alloc(module->mempool, SemaGeneric);
            memcpy(copy, generic->generic, sizeof(SemaGeneric));
            copy->additional_params = vec_new_in(module->mempool, SemaType*);
            vec_extend(copy->additional_params, generic->additional_params);
            vec_extend(copy->additional_params, input);
            return sema_value_new_generic(module->mempool, copy);
        }
    }
    UNREACHABLE;
}

SemaGeneric *sema_generic_new_type(Mempool *mempool, SemaModule *module, Slice name, SemaType **params)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_TYPE;
        out->additional_params = vec_new_in(mempool, SemaType*);
        out->gen_params = params;
        out->name = name;
        out->module = module;
    )

void sema_generic_type_add_extension(SemaModule *module, SemaGeneric *generic, Slice name, SemaExtDecl decl) {
    if (generic->kind != SEMA_GENERIC_TYPE) {
        sema_module_err(module, name, "only type generics can be extended");
        return;
    }
    sema_type_alias_add_extension(module, generic->type->alias, name, decl);
}

SemaType *sema_generic_type_generate(SemaGeneric *generic, SemaType **inputs) {
    assert(generic->kind == SEMA_GENERIC_TYPE);
    assert(sema_generic_input_count(generic) == vec_len(inputs));
    return sema_type_replace(generic->module->mempool, generic->type, generic->gen_params,
        sema_generic_get_input(generic, inputs));
}

bool sema_generic_type_search_ext(SemaGeneric *generic, SemaType **inputs, Slice name, SemaExtDecl *output) {
    assert(generic->kind == SEMA_GENERIC_TYPE);
    SemaExtDecl *decl = keymap_get(generic->type->alias->decls_map, name);
    if (decl) {
        *output = *decl;
        SemaGeneric *generic = sema_value_is_generic(output->function);
        assert(generic);
        output->function = sema_generate(generic, inputs);
        return true;
    }
    return sema_type_search_ext(generic->module, sema_type_replace(generic->module->mempool, generic->type,
        generic->gen_params, sema_generic_get_input(generic, inputs)), name, output);
}

SemaGeneric *sema_generic_new_generic(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, SemaGeneric *generic)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_GENERIC;
        out->additional_params = vec_new_in(mempool, SemaType*);
        out->gen_params = params;
        out->name = name;
        out->module = module;
        out->generic = generic;
    )

SemaGeneric *sema_generic_new_func(Mempool *mempool, SemaModule *module, Slice name, SemaType **params, HirGenScopeId scope)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_FUNC;
        out->additional_params = vec_new_in(mempool, SemaType*);
        out->gen_params = params;
        out->name = name;
        out->module = module;
        out->func.scope = scope;
    )
