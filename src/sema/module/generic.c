#include "generic.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include "sema/module/const.h"

SemaValue *sema_generate(SemaGeneric *generic, SemaType **input) {
    assert(vec_len(input) == vec_len(generic->params));
    switch (generic->kind) {
        case SEMA_GENERIC_TYPE:
            return sema_value_new_type(generic->module->mempool, sema_type_new_generate(
                    generic->module, generic, input));
        case SEMA_GENERIC_FUNC: {
            HirTypeId *params = vec_new_in(generic->module->mempool, HirTypeId);
            vec_resize(params, vec_len(input));
            for (size_t i = 0; i < vec_len(input); i++) {
                params[i] = sema_type_hir_id(input[i]);
            }
            return sema_value_new_runtime_const(generic->module->mempool, sema_const_new_gen_func(generic->module->mempool,
                sema_type_generate(generic->module, generic->func.type, generic->params, input), generic->func.scope,
                    generic->func.id, params));
        }
    }
    UNREACHABLE;
}

SemaGeneric *sema_generic_new_type(Mempool *mempool, SemaModule *module, SemaType **params)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_TYPE;
        out->params = params;
        out->module = module;
    )

SemaGeneric *sema_generic_new_func(Mempool *mempool, SemaModule *module, SemaType **params, HirGenScopeId scope)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_FUNC;
        out->params = params;
        out->module = module;
        out->func.scope = scope;
    )
