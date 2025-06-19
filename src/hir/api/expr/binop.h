#pragma once

#include "hir/api/type.h"
#include <stddef.h>

typedef enum {
    HIR_NUMBER_INT,
    HIR_NUMBER_UINT,
    HIR_NUMBER_FLOAT,
} HirNumberInfoKind;

typedef struct {
    HirNumberInfoKind kind;

    union {
        HirTypeIntSize int_size;
        HirTypeFloatSize float_size;
    };
} HirNumberInfo;

static inline HirNumberInfo hir_number_info_new_float(HirTypeFloatSize size) {
    HirNumberInfo info = {
        .kind = HIR_NUMBER_FLOAT,
        .float_size = size
    };
    return info;
}

static inline HirNumberInfo hir_number_info_new_int(bool is_signed, HirTypeIntSize size) {
    HirNumberInfo info = {
        .kind = is_signed ? HIR_NUMBER_INT : HIR_NUMBER_UINT,
        .int_size = size
    };
    return info;
}

typedef enum {
    HIR_BINOP_ARITHM_ADD,
    HIR_BINOP_ARITHM_SUB,
    HIR_BINOP_ARITHM_MUL,
    HIR_BINOP_ARITHM_DIV,
} HirBinopArithmeticKind;

typedef struct {
    HirBinopArithmeticKind kind;
    HirNumberInfo number;
} HirBinopArithmetic;

typedef enum {
    HIR_BINOP_ORDER_LT, HIR_BINOP_ORDER_GT,
    HIR_BINOP_ORDER_LE, HIR_BINOP_ORDER_GE,
} HirBinopOrderKind;

typedef struct {
    HirBinopOrderKind kind;
    HirNumberInfo number;
} HirBinopOrder;

typedef enum {
    HIR_COMPARE_EQ,
    HIR_COMPARE_NE,
} HirBinopCompareKind;

typedef enum {
    HIR_COMPARE_NUMBER,
    HIR_COMPARE_BOOL,
} HirBinopCompareValueKind;

typedef struct {
    HirBinopCompareKind kind;
    HirBinopCompareValueKind val_kind;

    union {
        HirNumberInfo number;
    };
} HirBinopCompare;

typedef enum {
    HIR_BINOP_BOOL_OR,
    HIR_BINOP_BOOL_AND,
} HirBinopBoolKind;

typedef struct {
    HirBinopBoolKind kind;
} HirBinopBool;

typedef enum {
    HIR_BINOP_INT_MOD,
    HIR_BINOP_INT_BITOR,
    HIR_BINOP_INT_BITAND,
    HIR_BINOP_INT_SHR,
    HIR_BINOP_INT_SHL,
} HirBinopIntKind;

typedef struct {
    HirBinopIntKind kind;
} HirBinopInt;

typedef enum {
    HIR_BINOP_ARITHMETIC,
    HIR_BINOP_ORDER,
    HIR_BINOP_COMPARE,
    HIR_BINOP_INT,
    HIR_BINOP_BOOL,
} HirBinopKind;

typedef struct {
    HirBinopKind kind;
    size_t ls, rs;

    union {
        HirBinopArithmetic arithm;
        HirBinopOrder order;
        HirBinopCompare compare;
        HirBinopInt integer;
        HirBinopBool boolean;
    };
} HirBinop;

static inline HirBinop hir_expr_binop_new_integer(size_t ls, size_t rs, HirBinopIntKind kind) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_INT,
        .integer = {
            .kind = kind,
        }
    };
    return binop;
}

static inline HirBinop hir_expr_binop_new_boolean(size_t ls, size_t rs, HirBinopBoolKind kind) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_BOOL,
        .boolean = {
            .kind = kind,
        }
    };
    return binop;
}

static inline HirBinop hir_expr_binop_new_compare_number(size_t ls, size_t rs, HirBinopCompareKind kind, HirNumberInfo number) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_COMPARE,
        .compare = {
            .kind = kind,
            .val_kind = HIR_COMPARE_NUMBER,
            .number = number
        }
    };
    return binop;
}

static inline HirBinop hir_expr_binop_new_compare_bool(size_t ls, size_t rs, HirBinopCompareKind kind) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_COMPARE,
        .compare = {
            .kind = kind,
            .val_kind = HIR_COMPARE_BOOL
        }
    };
    return binop;
}

static inline HirBinop hir_expr_binop_new_order(size_t ls, size_t rs, HirBinopOrderKind kind, HirNumberInfo number) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_ORDER,
        .order = {
            .kind = kind,
            .number = number
        }
    };
    return binop;
}

static inline HirBinop hir_expr_binop_new_arithmetic(size_t ls, size_t rs, HirBinopArithmeticKind kind, HirNumberInfo number) {
    HirBinop binop = {
        .ls = ls,
        .rs = rs,
        .kind = HIR_BINOP_ARITHMETIC,
        .arithm = {
            .kind = kind,
            .number = number
        }
    };
    return binop;
}
