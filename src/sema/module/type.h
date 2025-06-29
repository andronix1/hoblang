#pragma once

#include "hir/api/type.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include <stdbool.h>

typedef struct {
    SemaValue *function;
    SemaModule *module;
    bool by_ref;
} SemaExtDecl;

static inline SemaExtDecl sema_alias_decl_new(SemaValue *function, SemaModule *module, bool by_ref) {
    SemaExtDecl decl = {
        .function = function,
        .module = module,
        .by_ref = by_ref,
    };
    return decl;
}

typedef struct SemaTypeAlias {
    Slice name;
    SemaExtDecl *decls_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, Slice name);

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
    SEMA_TYPE_GEN_PARAM,
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

    SemaTypeAlias *alias;

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

        struct {
            HirGenParamId id;
            Slice name;
        } gen_param;

        Slice generic_name;
    };
} SemaType;

HirTypeIntSize sema_type_int_size_to_hir(SemaTypeIntSize size);
HirTypeFloatSize sema_type_float_size_to_hir(SemaTypeFloatSize size);

SemaType *sema_type_new_void(Mempool *mempool);
SemaType *sema_type_new_bool(Mempool *mempool);
SemaType *sema_type_new_record(Mempool *mempool, SemaModule *module, size_t type_id);
SemaType *sema_type_new_gen_param(Mempool *mempool, Slice name, HirGenParamId gen_param);
SemaType *sema_type_new_structure(Mempool *mempool, SemaTypeStructField *fields);
SemaType *sema_type_new_int(Mempool *mempool, SemaTypeIntSize size, bool is_signed);
SemaType *sema_type_new_float(Mempool *mempool, SemaTypeFloatSize size);
SemaType *sema_type_new_pointer(Mempool *mempool, SemaType *pointer_to);
SemaType *sema_type_new_function(Mempool *mempool, SemaType **args, SemaType *returns);
SemaType *sema_type_new_array(Mempool *mempool, size_t length, SemaType *of);
SemaType *sema_type_new_generic(Mempool *mempool, Slice name);
SemaType *sema_type_new_generate(Mempool *mempool, SemaGeneric *generic, SemaType **params);

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias);

bool sema_type_search_ext(SemaModule *module, SemaType *type, Slice name, SemaExtDecl *decl);
