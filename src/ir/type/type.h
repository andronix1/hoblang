#pragma once

#include <stdbool.h>
#include "core/mempool.h"
#include "ir/api/type.h"

typedef enum {
    IR_TYPE_VOID,
    IR_TYPE_BOOL,
    IR_TYPE_INT,
    IR_TYPE_FLOAT,
    IR_TYPE_FUNCTION,
    IR_TYPE_POINTER,
    IR_TYPE_ARRAY,
    IR_TYPE_STRUCT,
} IrTypeKind;

typedef enum {
    IR_TYPE_FLOAT_32,
    IR_TYPE_FLOAT_64,
} IrTypeFloatSize;

typedef enum {
    IR_TYPE_INT_8 = 0,
    IR_TYPE_INT_16,
    IR_TYPE_INT_32,
    IR_TYPE_INT_64,
} IrTypeIntSize;

typedef struct {
    IrTypeIntSize size;
    bool is_signed;
} IrTypeInt;

typedef struct {
    size_t length;
    IrTypeId of;
} IrTypeArray;

typedef struct {
    IrTypeId *fields;
} IrTypeStruct;

typedef struct {
    IrTypeId *args;
    IrTypeId returns;
} IrTypeFunction;

typedef struct IrType {
    IrTypeKind kind;

    union {
        IrTypeInt integer;
        IrTypeFloatSize float_size;
        IrTypeFunction function;
        IrTypeArray array;
        IrTypeId pointer_to;
        IrTypeStruct structure;
    };
} IrType;

IrTypeId *ir_type_depends_on(Mempool *mempool, IrType *type);

static inline IrType ir_type_new_void() {
    IrType type = { .kind = IR_TYPE_VOID };   
    return type;
}

static inline IrType ir_type_new_pointer(IrTypeId id) {
    IrType type = {
        .kind = IR_TYPE_POINTER,
        .pointer_to = id
    };   
    return type;
}

static inline IrType ir_type_new_array(IrTypeId of, size_t length) {
    IrType type = {
        .kind = IR_TYPE_ARRAY,
        .array = {
            .of = of,
            .length = length,
        }
    };
    return type;
}

static inline IrType ir_type_new_bool() {
    IrType type = { .kind = IR_TYPE_BOOL };   
    return type;
}

static inline IrType ir_type_new_int(IrTypeIntSize size, bool is_signed) {
    IrType type = {
        .kind = IR_TYPE_INT,
        .integer = {
            .size = size,
            .is_signed = is_signed
        }
    };
    return type;
}

static inline IrType ir_type_new_float(IrTypeFloatSize size) {
    IrType type = {
        .kind = IR_TYPE_FLOAT,
        .float_size = size,
    };
    return type;
}

static inline IrType ir_type_new_struct(IrTypeId *fields) {
    IrType type = {
        .kind = IR_TYPE_STRUCT,
        .structure = {
            .fields = fields,
        },
    };
    return type;
}
static inline IrType ir_type_new_function(IrTypeId *args, IrTypeId returns) {
    IrType type = {
        .kind = IR_TYPE_FUNCTION,
        .function = {
            .args = args,
            .returns = returns
        },
    };
    return type;
}
