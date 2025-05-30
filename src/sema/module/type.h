#pragma once

#include <stdbool.h>
#include "ast/api/alias.h"
#include "core/mempool.h"
#include "sema/module/api/type.h"

typedef enum {
    SEMA_PRIMITIVE_INT8,
    SEMA_PRIMITIVE_INT16,
    SEMA_PRIMITIVE_INT32,
    SEMA_PRIMITIVE_INT64,
} SemaPrimitiveIntSize;

typedef struct {
    SemaPrimitiveIntSize size;
    bool is_signed;
} SemaPrimitiveInt;

typedef enum {
    SEMA_PRIMITIVE_FLOAT32,
    SEMA_PRIMITIVE_FLOAT64,
} SemaPrimitiveFloat;

typedef enum {
    SEMA_PRIMITIVE_INT,
    SEMA_PRIMITIVE_FLOAT,
    SEMA_PRIMITIVE_VOID,
    SEMA_PRIMITIVE_BOOL,
} SemaPrimitiveKind;

typedef struct {
    SemaPrimitiveKind kind;

    union {
        SemaPrimitiveInt integer;
        SemaPrimitiveFloat fp;
    };
} SemaTypePrimitive;

typedef enum {
    SEMA_TYPE_PRIMITIVE,
    SEMA_TYPE_FUNCTION,
    SEMA_TYPE_POINTER,
    SEMA_TYPE_STRUCT,
    SEMA_TYPE_SLICE,
    SEMA_TYPE_GENERIC,
} SemaTypeKind;

typedef struct {
    SemaType *returns;
    SemaType **args;
} SemaTypeFunction;

typedef struct {
    bool is_local;
    SemaType *type;
} SemaTypeStructField;

typedef struct {
    SemaTypeStructField *fields_map;
} SemaTypeStruct;

typedef struct {
    SemaType **variants;
} SemaTypeGeneric;

typedef struct SemaType {
    SemaTypeKind kind;
    SemaTypeAlias *alias;

    union {
        SemaTypePrimitive primitive;
        SemaTypeFunction function;
        SemaTypeStruct structure;
        SemaType *pointer_to;
        SemaType *slice_of;
        SemaTypeGeneric generic;
    };
} SemaType;

SemaType *sema_type_new_primitive_int(Mempool *mempool, SemaPrimitiveIntSize size, bool is_signed);
SemaType *sema_type_new_primitive_float(Mempool *mempool, SemaPrimitiveFloat kind);
SemaType *sema_type_new_primitive_void(Mempool *mempool);
SemaType *sema_type_new_primitive_bool(Mempool *mempool);

SemaTypeStructField sema_type_struct_field_new(bool is_local, SemaType *type);
SemaType *sema_type_new_struct(Mempool *mempool, SemaTypeStructField *fields_map);

SemaType *sema_type_new_function(Mempool *mempool, SemaType **args, SemaType *returns);
SemaType *sema_type_new_pointer(Mempool *mempool, SemaType *to);
SemaType *sema_type_new_slice(Mempool *mempool, SemaType *of);
SemaType *sema_type_new_generic(Mempool *mempool);

SemaType *sema_type_new_alias(Mempool *mempool, SemaTypeAlias *alias, const SemaType *type);
