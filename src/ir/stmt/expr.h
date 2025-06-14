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

#include "ir/api/const.h"
#include "ir/api/decl.h"
#include "ir/api/type.h"
#include "ir/api/local.h"
#include <stdint.h>
#include "expr/binop.h"

typedef enum {
    IR_EXPR_STEP_CONST,
    IR_EXPR_STEP_BINOP,
    IR_EXPR_STEP_STRUCT_FIELD,
    IR_EXPR_STEP_GET_DECL,
    IR_EXPR_STEP_GET_LOCAL,
    IR_EXPR_STEP_CALL,
    IR_EXPR_STEP_BOOL_SKIP,
    IR_EXPR_STEP_TAKE_REF,
    IR_EXPR_STEP_DEREF,
    IR_EXPR_STEP_CAST_INT,
    IR_EXPR_STEP_NOT,
    IR_EXPR_STEP_BUILD_STRUCT,
    IR_EXPR_STEP_STRING,
    /*
    SEMA_PATH_SEGMENT_DECL,
    SEMA_PATH_SEGMENT_DEREF,
    */
} IrExprStepKind;

typedef struct {
    IrExprStepKind kind;

    IrTypeId type;

    union {
        IrConst *constant;

        IrBinop binop;

        struct {
            size_t idx;
            size_t step;
        } struct_field;

        IrLocalId local_id;
        IrDeclId decl_id;

        Slice string;

        struct {
            size_t callable;
            size_t *args;
        } call;

        struct {
            size_t step_id;
            IrTypeId source, dest;
        } cast_int;

        struct {
            size_t condition;
            bool expect;
            bool result;
        } bool_skip;

        struct {
            IrTypeId type;
            size_t *fields;
        } build_struct;

        size_t ref_step;
        size_t deref_step;
        size_t not_step;
    };
} IrExprStep;

static inline IrExprStep ir_expr_step_new_not(size_t step_id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_NOT,
        .not_step = step_id
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_string(Slice string) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_STRING,
        .string = string
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_bool_skip(size_t condition, bool expect, bool result) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_BOOL_SKIP,
        .bool_skip = {
            .condition = condition,
            .expect = expect,
            .result = result 
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_cast_int(size_t step_id, IrTypeId source, IrTypeId dst) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_CAST_INT,
        .cast_int = {
            .step_id = step_id,
            .source = source,
            .dest = dst
        }
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_deref(size_t step_id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_DEREF,
        .deref_step = step_id
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_take_ref(size_t step_id) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_TAKE_REF,
        .ref_step = step_id
    };
    return step;
}

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

static inline IrExprStep ir_expr_step_new_const(IrConst *constant) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_CONST,
        .constant = constant 
    };
    return step;
}

static inline IrExprStep ir_expr_step_new_build_struct(IrTypeId type_id, size_t *fields) {
    IrExprStep step = {
        .kind = IR_EXPR_STEP_BUILD_STRUCT,
        .build_struct = {
            .type = type_id,
            .fields = fields
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
