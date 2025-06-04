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

#include "ir/api/decl.h"
#include "ir/api/type.h"
#include "ir/type/type.h"
#include "ir/api/local.h"
#include <stdint.h>
#include "expr/binop.h"

typedef enum {
    IR_EXPR_STEP_INT,
    IR_EXPR_STEP_REAL,
    IR_EXPR_STEP_BINOP,
    IR_EXPR_STEP_STRUCT_FIELD,
    IR_EXPR_STEP_GET_DECL,
    IR_EXPR_STEP_GET_LOCAL,
    IR_EXPR_STEP_CALL,
    /*
    SEMA_PATH_SEGMENT_DECL,
    SEMA_PATH_SEGMENT_DEREF,
    */
} IrExprStepKind;

typedef struct {
    IrExprStepKind kind;

    IrTypeId type;

    union {
        struct {
            IrTypeId type;
            uint64_t value;
        } integer;

        struct {
            IrTypeId type;
            long double value;
        } real;

        IrBinop binop;

        struct {
            size_t idx;
            size_t step;
        } struct_field;

        IrLocalId local_id;
        IrDeclId decl_id;

        struct {
            size_t callable;
            size_t *args;
        } call;

    };
} IrExprStep;

static inline IrExprStep ir_expr_step_new_call(size_t *args, size_t callable) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_CALL,
        .call = {
            .args = args,
            .callable = callable
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_get_decl(IrDeclId id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_GET_DECL,
        .decl_id = id
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_get_local(IrLocalId id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_GET_LOCAL,
        .local_id = id
    };
    return step;
}

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

static inline IrExprStep ir_expr_step_new_real(IrTypeId type, long double value) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_REAL,
        .real = {
            .type = type,
            .value = value
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_int(IrTypeId type_id, uint64_t value) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_INT,
        .integer = {
            .type = type_id,
            .value = value
        }
    };
    return step;
}

typedef struct IrExpr {
    IrExprStep *steps;
} IrExpr;

static inline IrExpr ir_expr_new(IrExprStep *steps) {
    IrExpr expr = { .steps = steps };
    return expr;
}
