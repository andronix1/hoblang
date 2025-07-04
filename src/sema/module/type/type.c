#include "type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "hir/api/type.h"
#include "sema/module/api/type.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "sema/module/api/module.h"
#include <stdio.h>
#include <string.h>

HirTypeFloatSize sema_type_float_size_to_hir(SemaTypeFloatSize size) {
    switch (size) {
        case SEMA_FLOAT_32: return HIR_TYPE_FLOAT_32;
        case SEMA_FLOAT_64: return HIR_TYPE_FLOAT_64;
    }
    UNREACHABLE;
}

HirTypeIntSize sema_type_int_size_to_hir(SemaTypeIntSize size) {
    switch (size) {
        case SEMA_INT_8: return HIR_TYPE_INT_8;
        case SEMA_INT_16: return HIR_TYPE_INT_16;
        case SEMA_INT_32: return HIR_TYPE_INT_32;
        case SEMA_INT_64: return HIR_TYPE_INT_64;
    }
    UNREACHABLE;
}

#define SEMA_TYPE_CONSTRUCT(KIND, FIELDS) { \
        SemaType *out = mempool_alloc(mempool, SemaType); \
        out->kind = KIND; \
        out->alias = NULL; \
        FIELDS; \
        return out; \
    }

SemaType *sema_type_new_void(Mempool *mempool)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_VOID,)

SemaType *sema_type_new_record(Mempool *mempool, SemaModule *module, size_t type_id)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_RECORD,
        out->record.id = type_id;
        out->record.module = module;
    )

SemaType *sema_type_new_structure(Mempool *mempool, SemaTypeStructField *fields_map)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_STRUCTURE, out->structure.fields_map = fields_map;)

SemaType *sema_type_new_bool(Mempool *mempool)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_BOOL,)

SemaType *sema_type_new_int(Mempool *mempool, SemaTypeIntSize size, bool is_signed)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_INT,
        out->integer.size = size;
        out->integer.is_signed = is_signed;
    )

SemaType *sema_type_new_float(Mempool *mempool, SemaTypeFloatSize size)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_FLOAT, out->float_size = size)

SemaType *sema_type_new_array(Mempool *mempool, size_t length, SemaType *of)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_ARRAY,
        out->array.of = of;
        out->array.length = length;
    )

SemaType *sema_type_new_pointer(Mempool *mempool, SemaType *pointer_to)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_POINTER, out->pointer_to = pointer_to)

SemaType *sema_type_new_function(Mempool *mempool, SemaType **args, SemaType *returns)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_FUNCTION,
        out->function.args = args;
        out->function.returns = returns;
    )

SemaType *sema_type_new_enum(Mempool *mempool, SemaType *type, SemaEnumVariant *variants_map)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_ENUM,
        out->enumeration.variants_map = variants_map;
        out->enumeration.type = type;
    )

SemaType *sema_type_new_generic(Mempool *mempool, Slice name)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GENERIC, out->generic_name = name)

SemaType *sema_type_new_generate(Mempool *mempool, SemaGeneric *generic, SemaType **params)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GENERATE,
        out->generate.generic = generic;
        out->generate.params = params;
        out->generate.cache = NULL;
    )

SemaType *sema_type_new_gen_param(Mempool *mempool, Slice name, HirGenParamId gen_param)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GEN_PARAM,
        out->gen_param.id = gen_param;
        out->gen_param.name = name;
    )

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias) {
    SemaType *result = mempool_alloc(mempool, SemaType);
    memcpy(result, type, sizeof(SemaType));
    assert(!type->alias);
    result->alias = alias;
    return result;
}

SemaType *sema_type_generate(SemaType *type) {
    assert(type->kind == SEMA_TYPE_GENERATE);
    if (type->generate.cache) return type->generate.cache;
    return type->generate.cache = sema_generic_type_generate(type->generate.generic, type->generate.params);
}

SemaType *sema_type_get_record(SemaType *type) {
    assert(type->kind == SEMA_TYPE_RECORD);
    return type->record.module->types[type->record.id];
}

static inline bool sema_type_search_primary_ext(SemaModule *module, SemaType *type, Slice name, SemaExtDecl *output) {
    if (type->kind == SEMA_TYPE_GENERATE) {
        SemaGeneric *generic = type->generate.generic;
        assert(generic->kind == SEMA_GENERIC_TYPE);
        SemaExtDecl *decl = keymap_get(generic->type->alias->decls_map, name);
        if (decl) {
            *output = *decl;
            SemaGeneric *generic = sema_value_is_generic(output->function);
            assert(generic);
            output->function = sema_generate(generic, type->generate.params);
            return true;
        }
        return sema_type_search_ext(module, sema_type_generate(type), name, output);
    }
    if (!type->alias) {
        return false;
    }
    SemaExtDecl *decl = keymap_get(type->alias->decls_map, name);
    if (decl) {
        *output = *decl;
        return true;
    }
    if (type->kind == SEMA_TYPE_RECORD) {
        return sema_type_search_ext(module, sema_type_get_record(type), name, output);
    }
    return false;
}

static inline bool sema_type_search_all_ext(SemaModule *module, SemaType *type, Slice name, SemaExtDecl *output) {
    if (sema_type_search_primary_ext(module, type, name, output)) {
        return true;
    }
    if (type->kind == SEMA_TYPE_POINTER) {
        if (sema_type_search_primary_ext(module, type->pointer_to, name, output) && output->by_ref) {
            output->by_ref = false;
            return true;
        }
    }
    return false;
}

bool sema_type_search_ext(SemaModule *module, SemaType *type, Slice name, SemaExtDecl *output) {
    if (sema_type_search_all_ext(module, type, name, output)) {
        if (output->module != NULL && output->module != module) {
            sema_module_err(module, name, "`$S` is private", name);
            return false;
        }
        return true;
    }
    return false;
}
