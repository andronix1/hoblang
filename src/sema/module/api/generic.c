#include "generic.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/generic.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static inline SemaType *sema_type_generate(Mempool *mempool, SemaType *type, SemaGenericParam *params, SemaType **iparams) {
    for (size_t i = 0; i < vec_len(params); i++) {
        if (params->type == type) {
            return iparams[i];
        }
    }
    switch (type->kind) {
        case SEMA_TYPE_PRIMITIVE: return type;
        case SEMA_TYPE_FUNCTION: {
            SemaType **args = vec_new_in(mempool, SemaType*);
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                vec_push(args, sema_type_generate(mempool, type->function.args[i], params, iparams));
            }
            return sema_type_new_function(mempool, args,
                sema_type_generate(mempool, type->function.returns, params, iparams));
        }
        case SEMA_TYPE_POINTER:
            return sema_type_new_pointer(mempool, sema_type_generate(mempool, type->pointer_to,
                params, iparams));
        case SEMA_TYPE_SLICE:
            return sema_type_new_slice(mempool, sema_type_generate(mempool, type->slice_of,
                params, iparams));
        case SEMA_TYPE_STRUCT: {
            SemaTypeStructField *fields_map = keymap_new_in(mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                keymap_insert(fields_map, field->key, sema_type_struct_field_new(
                    field->value.is_local, sema_type_generate(mempool, field->value.type,
                        params, iparams)));
            }
            return sema_type_new_struct(mempool, fields_map);
        }
        case SEMA_TYPE_GENERIC: UNREACHABLE;
    }
    UNREACHABLE;
}

SemaValue *sema_value_generate(Mempool *mempool, SemaGeneric *generic, SemaType **params) {
    assert(vec_len(generic->params) == vec_len(params));
    switch (generic->kind) {
        case SEMA_GENERIC_TYPE:
            return sema_value_new_type(mempool, sema_type_generate(mempool, generic->type, generic->params, params));
    }
    UNREACHABLE;
}

SemaType *sema_generic_is_type(SemaGeneric *generic) {
    if (generic->kind != SEMA_GENERIC_TYPE) {
        return NULL;
    }
    return generic->type;
}
