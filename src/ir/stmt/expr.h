#pragma once

/*
 1 + 2 * 3 / (1 + 2 * 3)
 --+--------------------
 0 | 1
 1 | 2
 2 | 3
 3 | [1] * [2]
 4 | 1
 5 | 2
 6 | 3
 7 | [5] * [6]
 8 | [4] + [7]
 9 | [3] / [8]
10 | [0] + [9]

AST_EXPR_PATH,
AST_EXPR_INTEGER,
AST_EXPR_STRING,
AST_EXPR_CALL,
AST_EXPR_SCOPE,
AST_EXPR_BINOP,
AST_EXPR_STRUCT,
*/

#include "ir/type/type.h"
#include <stdint.h>
#include "expr/binop.h"

typedef enum {
    IR_EXPR_STEP_INT,
    IR_EXPR_STEP_REAL,
    IR_EXPR_STEP_BINOP,
    IR_EXPR_STEP_STRUCT_FIELD,
    /*
    IR_EXPR_STEP_CALL,
    SEMA_PATH_SEGMENT_DECL,
    SEMA_PATH_SEGMENT_DEREF,
    */
} IrExprStepKind;

typedef struct {
    IrExprStepKind kind;

    union {
        struct {
            IrTypeInt type;
            uint64_t value;
        } integer;

        struct {
            IrTypeFloatSize size;
            long double value;
        } real;

        IrBinop binop;

        struct {
            size_t idx;
            size_t step;
        } struct_field;
    };
} IrExprStep;

static inline IrExprStep ir_expr_step_new_struct_field(size_t idx, size_t step_id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_STRUCT_FIELD,
        .struct_field = {
            .idx = idx,
            .step = step_id,
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_binop(IrBinop binop) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_BINOP,
        .binop = binop
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_real(IrTypeFloatSize size, long double value) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_REAL,
        .real = {
            .size = size,
            .value = value
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_int(IrTypeIntSize size, bool is_signed, uint64_t value) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_INT,
        .integer = {
            .type = {
                .size = size,
                .is_signed = is_signed
            },
            .value = value
        }
    };
    return step;
}

typedef struct {
    IrExprStep *steps;
} IrExpr;

static inline IrExpr ir_expr_new(IrExprStep *steps) {
    IrExpr expr = { .steps = steps };
    return expr;
}
