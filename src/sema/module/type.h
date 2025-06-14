#pragma once

#include "ir/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include <stdbool.h>

typedef struct SemaTypeAlias {
    IrTypeId id;
    SemaDecl **decls_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, IrTypeId id);

typedef enum {
    SEMA_INT_8,
    SEMA_INT_16,
    SEMA_INT_32,
    SEMA_INT_64,
} SemaTypeIntSize;

typedef enum {
    SEMA_TYPE_VOID,
    SEMA_TYPE_INT,
    SEMA_TYPE_BOOL,
    SEMA_TYPE_FUNCTION,
    SEMA_TYPE_POINTER,
    SEMA_TYPE_STRUCTURE,
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
    IrTypeId ir_id;

    union {
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

        SemaType *pointer_to;

        struct {
            SemaTypeId id;
            SemaModule *module;
        } record;
    };
} SemaType;

IrTypeIntSize sema_type_int_size_to_ir(SemaTypeIntSize size);

SemaType *sema_type_new_void(SemaModule *module);
SemaType *sema_type_new_bool(SemaModule *module);
SemaType *sema_type_new_record(SemaModule *module, size_t type_id);
SemaType *sema_type_new_structure(SemaModule *module, SemaTypeStructField *fields);
SemaType *sema_type_new_int(SemaModule *module, SemaTypeIntSize size, bool is_signed);
SemaType *sema_type_new_pointer(SemaModule *module, SemaType *pointer_to);
SemaType *sema_type_new_function(SemaModule *module, SemaType **args, SemaType *returns);

SemaType *sema_type_new_alias(Mempool *mempool, SemaType *type, SemaTypeAlias *alias);
