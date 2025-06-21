#pragma once

#include "hir/api/gen.h"
#include <stdbool.h>
#include <stddef.h>

typedef size_t HirTypeId;

typedef enum {
    HIR_TYPE_VOID,
    HIR_TYPE_BOOL,
    HIR_TYPE_INT,
    HIR_TYPE_FLOAT,
    HIR_TYPE_FUNCTION,
    HIR_TYPE_POINTER,
    HIR_TYPE_ARRAY,
    HIR_TYPE_STRUCT,
    HIR_TYPE_GEN,
} HirTypeKind;

typedef enum {
    HIR_TYPE_FLOAT_32,
    HIR_TYPE_FLOAT_64,
} HirTypeFloatSize;

typedef enum {
    HIR_TYPE_INT_8 = 0,
    HIR_TYPE_INT_16,
    HIR_TYPE_INT_32,
    HIR_TYPE_INT_64,
} HirTypeIntSize;

typedef struct {
    HirTypeIntSize size;
    bool is_signed;
} HirTypeInt;

typedef struct {
    HirTypeId of;
    size_t length;
} HirTypeArray;

typedef struct {
    HirTypeId type;
} HirTypeStructField;

static inline HirTypeStructField hir_type_struct_field_new(HirTypeId type) {
    HirTypeStructField field = { .type = type };
    return field;
}

typedef struct {
    HirTypeStructField *fields;
} HirTypeStruct;

typedef struct {
    HirTypeId *args;
    HirTypeId returns;
} HirTypeFunction;

typedef struct HirType {
    HirTypeKind kind;

    union {
        HirTypeInt integer;
        HirTypeFloatSize float_size;
        HirTypeFunction function;
        HirTypeArray array;
        HirTypeId pointer_to;
        HirTypeStruct structure;
        HirGenParamId gen_param;
    };
} HirType;

bool hir_type_eq(const HirType *a, const HirType *b);

static inline HirType hir_type_new_gen(HirGenParamId gen_param) {
    HirType type = {
        .kind = HIR_TYPE_GEN,
        .gen_param = gen_param
    };
    return type;
}

static inline HirType hir_type_new_void() {
    HirType type = { .kind = HIR_TYPE_VOID };
    return type;
}

static inline HirType hir_type_new_pointer(HirTypeId id) {
    HirType type = {
        .kind = HIR_TYPE_POINTER,
        .pointer_to = id
    };   
    return type;
}

static inline HirType hir_type_new_array(HirTypeId of, size_t length) {
    HirType type = {
        .kind = HIR_TYPE_ARRAY,
        .array = {
            .of = of,
            .length = length,
        }
    };
    return type;
}

static inline HirType hir_type_new_bool() {
    HirType type = { .kind = HIR_TYPE_BOOL };   
    return type;
}

static inline HirType hir_type_new_int(HirTypeIntSize size, bool is_signed) {
    HirType type = {
        .kind = HIR_TYPE_INT,
        .integer = {
            .size = size,
            .is_signed = is_signed
        }
    };
    return type;
}

static inline HirType hir_type_new_float(HirTypeFloatSize size) {
    HirType type = {
        .kind = HIR_TYPE_FLOAT,
        .float_size = size,
    };
    return type;
}

static inline HirType hir_type_new_struct(HirTypeStructField *fields) {
    HirType type = {
        .kind = HIR_TYPE_STRUCT,
        .structure = {
            .fields = fields,
        },
    };
    return type;
}

static inline HirType hir_type_new_function(HirTypeId *args, HirTypeId returns) {
    HirType type = {
        .kind = HIR_TYPE_FUNCTION,
        .function = {
            .args = args,
            .returns = returns
        },
    };
    return type;
}
