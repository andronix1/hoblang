#include "params.h"
#include "core/vec.h"
#include "sema/module/type.h"
#include "sema/module/api/type.h"

void sema_generic_param_add_variant(SemaGenericParam *param, SemaType *type) {
    if (param->type == type) {
        return;
    }
    for (size_t i = 0; i < vec_len(param->type->generic.variants); i++) {
        if (sema_type_eq(param->type->generic.variants[i], type)) {
            return;
        }
    }
    vec_push(param->type->generic.variants, type);
}

SemaGenericParam sema_generic_param_new(Slice name, SemaType *type) {
    SemaGenericParam param = {
        .name = name,
        .type = type,
    };
    return param;
}
