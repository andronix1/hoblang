#include "type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api.h"
#include "sema/module/api/type.h"
#include "sema/module/generics/params.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include <assert.h>
#include <stdio.h>

static inline SemaGenericTypeUsage sema_generic_type_usage_new(SemaType **params, SemaType *type) {
    SemaGenericTypeUsage usage = {
        .params = params,
        .type = type
    };
    return usage;
}

SemaGenericTypeInfo sema_generic_type_info_new(Mempool *mempool, SemaGenericParam *params, SemaType *type) {
    SemaGenericTypeInfo info = {
        .type = type,
        .usages = vec_new_in(mempool, SemaGenericTypeUsage),
        .params = params,
        .locked = false
    };
    return info;
}

inline static bool sema_params_equals(SemaType **a, SemaType **b) {
    assert(vec_len(a) == vec_len(b));
    for (size_t i = 0; i < vec_len(a); i++) {
        if (!sema_type_eq(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

static inline SemaType *_sema_type_generate(Mempool *mempool, SemaType *type, SemaGenericParam *params, SemaType **iparams) {
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
                vec_push(args, _sema_type_generate(mempool, type->function.args[i], params, iparams));
            }
            return sema_type_new_function(mempool, args,
                _sema_type_generate(mempool, type->function.returns, params, iparams));
        }
        case SEMA_TYPE_POINTER:
            return sema_type_new_pointer(mempool, _sema_type_generate(mempool, type->pointer_to,
                params, iparams));
        case SEMA_TYPE_SLICE:
            return sema_type_new_slice(mempool, _sema_type_generate(mempool, type->slice_of,
                params, iparams));
        case SEMA_TYPE_STRUCT: {
            SemaTypeStructField *fields_map = keymap_new_in(mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                keymap_insert(fields_map, field->key, sema_type_struct_field_new(
                    field->value.is_local, _sema_type_generate(mempool, field->value.type,
                        params, iparams)));
            }
            return sema_type_new_struct(mempool, fields_map);
        }
        case SEMA_TYPE_GENERIC: UNREACHABLE;
    }
    UNREACHABLE;
}

static inline int sema_type_usage_search(SemaGenericTypeInfo *info, SemaType **iparams) {
    for (size_t i = 0; i < vec_len(info->usages); i++) {
        SemaGenericTypeUsage *usage = &info->usages[i];
        if (sema_params_equals(iparams, usage->params)) {
            return i;
        }
    }
    return -1;
}

SemaType *sema_type_generate(SemaModule *module, Slice where, SemaGenericTypeInfo *info, SemaGenericParam *params, SemaType **iparams) {
    int id = sema_type_usage_search(info, iparams);
    if (id != -1) {
        return info->usages[id].type;
    }
    if (info->locked) {
        sema_module_err(module, where, "recursive generic detected");
        return NULL;
    }
    info->locked = true;
    SemaType *type = _sema_type_generate(module->mempool, info->type, params, iparams);
    vec_push(info->usages, sema_generic_type_usage_new(iparams, type));
    info->locked = false;
    return type;
}

static inline int sema_generic_usage_get_generic_id(SemaGenericTypeUsage *usage) {
    for (size_t i = 0; i < vec_len(usage->params); i++) {
        if (usage->params[i]->kind == SEMA_TYPE_GENERIC) {
            return i;
        }
    }
    return -1;
}

static inline void sema_generic_type_add_usages(Mempool *mempool, SemaGenericTypeInfo *info, SemaType **params, SemaType **variants, size_t idx) {
    assert(idx < vec_len(params));
    for (size_t i = 0; i < vec_len(variants); i++) {
        SemaType *variant = variants[i];
        SemaType **iparams = vec_new_in(mempool, SemaType*);
        vec_reserve(iparams, vec_len(iparams));
        for (size_t j = 0; j < idx; j++) vec_push(iparams, params[j]);
        vec_push(iparams, variant);
        for (size_t j = idx + 1; j < vec_len(params); j++) vec_push(iparams, params[j]);

        if (sema_type_usage_search(info, iparams) == -1) {
            SemaType *type = _sema_type_generate(mempool, info->type, info->params, iparams);
            vec_push(info->usages, sema_generic_type_usage_new(iparams, type));
        }
    }
}

bool sema_generic_type_expand_usages(Mempool *mempool, SemaGenericTypeInfo *info) {
    bool found = false;
    for (size_t i = 0; i < vec_len(info->usages); i++) {
        SemaGenericTypeUsage *usage = &info->usages[i];
        int id = sema_generic_usage_get_generic_id(usage);
        if (id == -1) {
            continue;
        }
        sema_generic_type_add_usages(mempool, info, usage->params, usage->params[id]->generic.variants, id);
        vec_remove_at(info->usages, i);
        i--;
        found = true;
    }
    return found;
}
