#pragma once

#include "ir/type/type.h"
#include <stddef.h>

typedef enum {
    IR_NUMBER_INT,
    IR_NUMBER_UINT,
    IR_NUMBER_FLOAT,
} IrNumberInfoKind;

typedef struct {
    IrNumberInfoKind kind;

    union {
        IrTypeIntSize int_size;
        IrTypeFloatSize float_size;
    };
} IrNumberInfo;

static inline IrNumberInfo ir_number_info_new_float(IrTypeFloatSize size) {
    IrNumberInfo info = {
        .kind = IR_NUMBER_FLOAT,
        .float_size = size
    };
    return info;
}

static inline IrNumberInfo ir_number_info_new_int(bool is_signed, IrTypeIntSize size) {
    IrNumberInfo info = {
        .kind = is_signed ? IR_NUMBER_INT : IR_NUMBER_UINT,
        .int_size = size
    };
    return info;
}

typedef enum {
    IR_BINOP_ARITHM_ADD,
    IR_BINOP_ARITHM_SUB,
    IR_BINOP_ARITHM_MUL,
    IR_BINOP_ARITHM_DIV,
} IrBinopArithmeticKind;

typedef struct {
    IrBinopArithmeticKind kind;
    IrNumberInfo number;
} IrBinopArithmetic;

typedef enum {
    IR_BINOP_ORDER_LT, IR_BINOP_ORDER_GT,
    IR_BINOP_ORDER_LE, IR_BINOP_ORDER_GE,
} IrBinopOrderKind;

typedef struct {
    IrBinopOrderKind kind;
    IrNumberInfo number;
} IrBinopOrder;

typedef enum {
    IR_COMPARE_EQ,
    IR_COMPARE_NE,
} IrBinopCompareKind;

typedef enum {
    IR_COMPARE_NUMBER,
    IR_COMPARE_BOOL,
} IrBinopCompareValueKind;

typedef struct {
    IrBinopCompareKind kind;
    IrBinopCompareValueKind val_kind;

    union {
        IrNumberInfo number;
    };
} IrBinopCompare;

typedef enum {
    IR_BINOP_BOOL_OR,
    IR_BINOP_BOOL_AND,
} IrBinopBoolKind;

typedef struct {
    IrBinopBoolKind kind;
} IrBinopBool;

typedef enum {
    IR_BINOP_INT_MOD,
} IrBinopIntKind;

typedef struct {
    IrBinopIntKind kind;
} IrBinopInt;

typedef enum {
    IR_BINOP_ARITHMETIC,
    IR_BINOP_ORDER,
    IR_BINOP_COMPARE,
    IR_BINOP_INT,
    IR_BINOP_BOOL,
} IrBinopKind;

typedef struct {
    IrBinopKind kind;
    size_t ls, rs;

    union {
        IrBinopArithmetic arithm;
        IrBinopOrder order;
        IrBinopCompare compare;
        IrBinopInt integer;
        IrBinopBool boolean;
    };
} IrBinop;

static inline IrBinop ir_expr_binop_new_integer(size_t ls, size_t rs, IrBinopIntKind kind) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_INT,
        .integer = {
            .kind = kind,
        }
    };
    return binop;
}

static inline IrBinop ir_expr_binop_new_boolean(size_t ls, size_t rs, IrBinopBoolKind kind) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_BOOL,
        .boolean = {
            .kind = kind,
        }
    };
    return binop;
}

static inline IrBinop ir_expr_binop_new_compare_number(size_t ls, size_t rs, IrBinopCompareKind kind, IrNumberInfo number) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_COMPARE,
        .compare = {
            .kind = kind,
            .val_kind = IR_COMPARE_NUMBER,
            .number = number
        }
    };
    return binop;
}

static inline IrBinop ir_expr_binop_new_compare_bool(size_t ls, size_t rs, IrBinopCompareKind kind) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_COMPARE,
        .compare = {
            .kind = kind,
            .val_kind = IR_COMPARE_BOOL
        }
    };
    return binop;
}

static inline IrBinop ir_expr_binop_new_order(size_t ls, size_t rs, IrBinopOrderKind kind, IrNumberInfo number) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_ORDER,
        .order = {
            .kind = kind,
            .number = number
        }
    };
    return binop;
}

static inline IrBinop ir_expr_binop_new_arithmetic(size_t ls, size_t rs, IrBinopArithmeticKind kind, IrNumberInfo number) {
    IrBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = IR_BINOP_ARITHMETIC,
        .arithm = {
            .kind = kind,
            .number = number
        }
    };
    return binop;
}
