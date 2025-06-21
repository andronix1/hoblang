#include "type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/vec.h"
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

static inline HirType sema_type_to_hir(SemaModule* module, SemaType *type) {
    switch (type->kind) {
        case SEMA_TYPE_VOID: return hir_type_new_void();
        case SEMA_TYPE_INT: return hir_type_new_int(sema_type_int_size_to_hir(type->integer.size), type->integer.is_signed);
        case SEMA_TYPE_FLOAT: return hir_type_new_float(sema_type_float_size_to_hir(type->float_size));
        case SEMA_TYPE_BOOL: return hir_type_new_bool();
        case SEMA_TYPE_FUNCTION: {
            HirTypeId *args = vec_new_in(module->mempool, HirTypeId);
            vec_resize(args, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                args[i] = sema_type_hir_id(type->function.args[i]);
            }
            return hir_type_new_function(args, sema_type_hir_id(type->function.returns));
        }
        case SEMA_TYPE_POINTER: return hir_type_new_pointer(sema_type_hir_id(type->pointer_to));
        case SEMA_TYPE_STRUCTURE: {
            HirTypeStructField *fields = vec_new_in(module->mempool, HirTypeStructField);
            vec_resize(fields, vec_len(type->structure.fields_map));
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                fields[i] = hir_type_struct_field_new(sema_type_hir_id(field->value.type));
            }
            return hir_type_new_struct(fields);
        }
        case SEMA_TYPE_ARRAY: return hir_type_new_array(sema_type_hir_id(type->array.of), type->array.length);
        case SEMA_TYPE_RECORD: TODO;

        case SEMA_TYPE_GENERATE: {
            SemaType *generated = sema_value_is_type(sema_generate(type->generate.generic, type->generate.params));
            assert(generated);
            return sema_type_to_hir(module, generated);
        }
        case SEMA_TYPE_GENERIC: UNREACHABLE;
    }
    UNREACHABLE;
}

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, Slice name, HirTypeId id)
    MEMPOOL_CONSTRUCT(SemaTypeAlias,
        out->id = id;
        out->name = name;
        out->decls_map = keymap_new_in(mempool, SemaDecl*);
    )

#define SEMA_TYPE_CONSTRUCT_SIMPLE(KIND, FIELDS) { \
        SemaType *out = mempool_alloc(module->mempool, SemaType); \
        out->kind = KIND; \
        FIELDS; \
        out->alias = NULL; \
        return out; \
    }

#define SEMA_TYPE_CONSTRUCT(KIND, FIELDS) { \
        SemaType *out = mempool_alloc(module->mempool, SemaType); \
        out->kind = KIND; \
        FIELDS; \
        out->alias = NULL; \
        out->hir_id = KIND == SEMA_TYPE_RECORD ? \
            sema_module_get_type_id(module, out->record.id) : \
            hir_add_type(module->hir, sema_type_to_hir(module, out)); \
        return out; \
    }

SemaType *sema_type_new_void(SemaModule *module)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_VOID,)

SemaType *sema_type_new_record(SemaModule *module, size_t type_id)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_RECORD,
        out->record.id = type_id;
        out->record.module = module;
    )

SemaType *sema_type_new_structure(SemaModule *module, SemaTypeStructField *fields_map)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_STRUCTURE, out->structure.fields_map = fields_map;)

SemaType *sema_type_new_bool(SemaModule *module)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_BOOL,)

SemaType *sema_type_new_int(SemaModule *module, SemaTypeIntSize size, bool is_signed)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_INT,
        out->integer.size = size;
        out->integer.is_signed = is_signed;
    )

SemaType *sema_type_new_float(SemaModule *module, SemaTypeFloatSize size)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_FLOAT, out->float_size = size)

SemaType *sema_type_new_array(SemaModule *module, size_t length, SemaType *of)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_ARRAY,
        out->array.of = of;
        out->array.length = length;
    )

SemaType *sema_type_new_pointer(SemaModule *module, SemaType *pointer_to)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_POINTER, out->pointer_to = pointer_to)

SemaType *sema_type_new_function(SemaModule *module, SemaType **args, SemaType *returns)
    SEMA_TYPE_CONSTRUCT(SEMA_TYPE_FUNCTION,
        out->function.args = args;
        out->function.returns = returns;
    )

SemaType *sema_type_new_generic(SemaModule *module)
    SEMA_TYPE_CONSTRUCT_SIMPLE(SEMA_TYPE_GENERIC,)

SemaType *sema_type_new_generate(SemaModule *module, SemaGeneric *generic, SemaType **params)
    SEMA_TYPE_CONSTRUCT_SIMPLE(SEMA_TYPE_GENERATE,
        out->generate.generic = generic;
        out->generate.params = params;
    )

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias) {
    SemaType *result = mempool_alloc(mempool, SemaType);
    memcpy(result, type, sizeof(SemaType));
    assert(!type->alias);
    result->alias = alias;
    return result;
}

