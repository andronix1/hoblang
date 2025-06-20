#pragma once

#include "hir/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include <stdbool.h>

typedef struct SemaTypeAlias {
    HirTypeId id;
    SemaDecl **decls_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, HirTypeId id);

typedef enum {
    SEMA_FLOAT_32,
    SEMA_FLOAT_64,
} SemaTypeFloatSize;

typedef enum {
    SEMA_INT_8,
    SEMA_INT_16,
    SEMA_INT_32,
    SEMA_INT_64,
} SemaTypeIntSize;

typedef enum {
    SEMA_TYPE_VOID,
    SEMA_TYPE_INT,
    SEMA_TYPE_FLOAT,
    SEMA_TYPE_BOOL,
    SEMA_TYPE_FUNCTION,
    SEMA_TYPE_POINTER,
    SEMA_TYPE_STRUCTURE,
    SEMA_TYPE_ARRAY,
    SEMA_TYPE_GENERIC,
    SEMA_TYPE_GENERATE,
    SEMA_TYPE_RECORD,
} SemaTypeKind;

typedef struct {
    SemaModule *module;
    SemaType *type;
} SemaTypeStructField;

static inline SemaTypeStructField sema_type_struct_field_new(SemaType *type, SemaModule *module) {
    SemaTypeStructField field = {
        .type = type,
        .module = module
    };
    return field;
}

typedef struct SemaType {
    SemaTypeKind kind;

    SemaTypeAlias **aliases;
    HirTypeId hir_id;

    union {
        SemaTypeFloatSize float_size;
        
        struct {
            SemaTypeIntSize size;
            bool is_signed;
        } integer;

        struct {
            SemaTypeStructField *fields_map;
        } structure;

        struct {
            SemaType **args;
            SemaType *returns;
        } function;

        struct {
            size_t length;
            SemaType *of;
        } array;

        SemaType *pointer_to;

        struct {
            SemaTypeId id;
            SemaModule *module;
        } record;

        struct {
            SemaGeneric *generic;
            SemaType **params;
        } generate;

        HirGenParamId gen_param_id;
    };
} SemaType;

HirTypeIntSize sema_type_int_size_to_hir(SemaTypeIntSize size);
HirTypeFloatSize sema_type_float_size_to_hir(SemaTypeFloatSize size);

SemaType *sema_type_new_void(SemaModule *module);
SemaType *sema_type_new_bool(SemaModule *module);
SemaType *sema_type_new_record(SemaModule *module, size_t type_id);
SemaType *sema_type_new_structure(SemaModule *module, SemaTypeStructField *fields);
SemaType *sema_type_new_int(SemaModule *module, SemaTypeIntSize size, bool is_signed);
SemaType *sema_type_new_float(SemaModule *module, SemaTypeFloatSize size);
SemaType *sema_type_new_pointer(SemaModule *module, SemaType *pointer_to);
SemaType *sema_type_new_function(SemaModule *module, SemaType **args, SemaType *returns);
SemaType *sema_type_new_array(SemaModule *module, size_t length, SemaType *of);
SemaType *sema_type_new_generic(SemaModule *module, HirGenParamId id);
SemaType *sema_type_new_generate(SemaModule *module, SemaGeneric *generic, SemaType **params);

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias);
