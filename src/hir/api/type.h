#pragma once

#include "core/mempool.h"
#include "hir/api/gen.h"
#include <stdbool.h>

typedef struct HirType HirType;

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
    HirType *of;
    size_t length;
} HirTypeArray;

typedef struct {
    HirType *type;
} HirTypeStructField;

static inline HirTypeStructField hir_type_struct_field_new(HirType *type) {
    HirTypeStructField field = { .type = type };
    return field;
}

typedef struct {
    HirTypeStructField *fields;
} HirTypeStruct;

typedef struct {
    HirType **args;
    HirType *returns;
} HirTypeFunction;

typedef struct HirType {
    HirTypeKind kind;

    union {
        HirTypeInt integer;
        HirTypeFloatSize float_size;
        HirTypeFunction function;
        HirTypeArray array;
        HirType *pointer_to;
        HirTypeStruct structure;
        HirGenParamId gen_param;
    };
} HirType;

bool hir_type_eq(const HirType *a, const HirType *b);

HirType *hir_type_new_gen(Mempool *mempool, HirGenParamId gen_param);
HirType *hir_type_new_void(Mempool *mempool);
HirType *hir_type_new_pointer(Mempool *mempool, HirType *to);
HirType *hir_type_new_array(Mempool *mempool, HirType *of, size_t length);
HirType *hir_type_new_bool(Mempool *mempool);
HirType *hir_type_new_int(Mempool *mempool, HirTypeIntSize size, bool is_signed);
HirType *hir_type_new_float(Mempool *mempool, HirTypeFloatSize size);
HirType *hir_type_new_struct(Mempool *mempool, HirTypeStructField *fields);
HirType *hir_type_new_function(Mempool *mempool, HirType **args, HirType *returns);
