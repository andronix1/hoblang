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

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, Slice name)
    MEMPOOL_CONSTRUCT(SemaTypeAlias,
        out->name = name;
        out->decls_map = keymap_new_in(mempool, SemaExtDecl);
    )

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

SemaType *sema_type_new_generic(Mempool *mempool)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GENERIC,)

SemaType *sema_type_new_generate(Mempool *mempool, SemaGeneric *generic, SemaType **params)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GENERATE,
        out->generate.generic = generic;
        out->generate.params = params;
    )

SemaType *sema_type_new_gen_param(Mempool *mempool, HirGenParamId gen_param)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_GEN_PARAM,
        out->gen_param = gen_param;
    )

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias) {
    SemaType *result = mempool_alloc(mempool, SemaType);
    memcpy(result, type, sizeof(SemaType));
    assert(!type->alias);
    result->alias = alias;
    return result;
}

bool sema_type_search_ext(SemaModule *module, SemaType *type, Slice name, SemaExtDecl *output) {
    if (type->kind == SEMA_TYPE_POINTER) {
        if (sema_type_search_ext(module, type->pointer_to, name, output) && output->by_ref) {
            output->by_ref = false;
            return true;
        }
    }
    if (type->kind == SEMA_TYPE_GENERATE) {
        SemaGeneric *generic = type->generate.generic;
        assert(generic->kind == SEMA_GENERIC_TYPE);
        SemaExtDecl *decl = keymap_get(generic->type.type->alias->decls_map, name);
        if (decl) {
            *output = *decl;
            SemaGeneric *generic = sema_value_is_generic(output->function);
            assert(generic);
            output->function = sema_generate(generic, type->generate.params);
            return true;
        }
        return sema_type_search_ext(module, sema_type_generate(module->mempool, generic->type.type,
            generic->gen_params, sema_generic_get_input(generic, type->generate.params)), name, output);
    }
    if (!type->alias) {
        return false;
    }
    SemaExtDecl *decl = keymap_get(type->alias->decls_map, name);
    if (decl) {
        if (decl->module != NULL && decl->module != module) {
            sema_module_err(module, name, "`$S` is private", name);
            return false;
        }
        *output = *decl;
        return true;
    }
    if (type->kind == SEMA_TYPE_RECORD) {
        return sema_type_search_ext(module, type->record.module->types[type->record.id], name, output);
    }
    return false;
}
