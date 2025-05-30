#include "generic.h"
#include "core/assert.h"
#include "core/vec.h"
#include "sema/module/api.h"
#include "sema/module/generic.h"
#include "sema/module/generics/params.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_value_generate(SemaModule *module, Slice where, SemaGeneric *generic, SemaType **params) {
    assert(vec_len(generic->params) == vec_len(params));
    for (size_t i = 0; i < vec_len(generic->params); i++) {
        sema_generic_param_add_variant(generic->params, params[i]);
    }
    switch (generic->kind) {
        case SEMA_GENERIC_TYPE:
            return sema_value_new_type(module->mempool, sema_type_generate(module,
                where, &generic->type, generic->params, params));
    }
    UNREACHABLE;
}

SemaType *sema_generic_is_type(SemaGeneric *generic) {
    if (generic->kind != SEMA_GENERIC_TYPE) {
        return NULL;
    }
    return generic->type.type;
}
