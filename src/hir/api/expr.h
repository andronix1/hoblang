#pragma once

#include "hir/api/const.h"
#include "hir/api/decl.h"
#include "hir/api/expr/binop.h"
#include "hir/api/func.h"
#include "hir/api/gen.h"
#include <stddef.h>

typedef size_t HirExprStepId;

typedef enum {
    HIR_EXPR_STEP_CONST,
    HIR_EXPR_STEP_BINOP,
    HIR_EXPR_STEP_STRUCT_FIELD,
    HIR_EXPR_STEP_GET_DECL,
    HIR_EXPR_STEP_GET_LOCAL,
    HIR_EXPR_STEP_CALL,
    HIR_EXPR_STEP_BOOL_SKIP,
    HIR_EXPR_STEP_TAKE_REF,
    HIR_EXPR_STEP_DEREF,
    HIR_EXPR_STEP_CAST_INT,
    HIR_EXPR_STEP_NOT,
    HIR_EXPR_STEP_BUILD_STRUCT,
    HIR_EXPR_STEP_BUILD_ARRAY,
    HIR_EXPR_STEP_IDX_ARRAY,
    HIR_EXPR_STEP_IDX_POINTER,
    HIR_EXPR_STEP_STRING,
    HIR_EXPR_STEP_SIZEOF,
    HIR_EXPR_STEP_NEG,
    HIR_EXPR_STEP_GEN_FUNC,
} HirExprStepKind;

typedef struct {
    HirExprStepKind kind;

    HirTypeId type;

    union {
        HirConst constant;

        HirBinop binop;

        struct {
            size_t idx;
            size_t step;
        } struct_field;

        struct {
            size_t *elements;
            HirTypeId type;
        } build_array;

        HirLocalId local_id;
        HirDeclId decl_id;

        struct {
            HirTypeId value;
            HirTypeId idx;
        } idx_pointer;

        struct {
            HirTypeId value;
            HirTypeId idx;
        } idx_array;

        struct {
            HirNumberInfo info;
            size_t step;
        } neg;

        struct {
            HirTypeId of;
            HirTypeId type;
        } size;

        Slice string;

        struct {
            size_t callable;
            size_t *args;
        } call;

        struct {
            size_t step_id;
            HirTypeId source, dest;
        } cast_int;

        struct {
            size_t condition;
            bool expect;
            bool result;
        } bool_skip;

        struct {
            HirTypeId type;
            size_t *fields;
        } build_struct;

        struct {
            HirTypeId type;
            HirTypeId *params;
            HirGenScopeId scope;
            HirGenFuncId func;

            HirGenUsageId usage;
        } gen_func;

        size_t ref_step;
        size_t deref_step;
        size_t not_step;
    };
} HirExprStep;

static inline HirExprStep hir_expr_step_new_gen_func(HirTypeId type, HirGenScopeId scope, HirGenFuncId func, HirTypeId *params) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_GEN_FUNC,
        .gen_func = {
            .type = type,
            .scope = scope,
            .func = func,
            .params = params,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_sizeof(HirTypeId of, HirTypeId type) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_SIZEOF,
        .size = {
            .type = type,
            .of = of,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_neg(size_t step_id, HirNumberInfo info) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_NEG,
        .neg = {
            .step = step_id,
            .info = info 
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_idx_pointer(size_t value, size_t idx) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_IDX_POINTER,
        .idx_pointer = {
            .value = value,
            .idx = idx
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_idx_array(size_t value, size_t idx) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_IDX_ARRAY,
        .idx_array = {
            .value = value,
            .idx = idx
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_not(size_t step_id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_NOT,
        .not_step = step_id
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_array(HirTypeId type, size_t *elements) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_BUILD_ARRAY,
        .build_array = {
            .elements = elements,
            .type = type
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_string(Slice string) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_STRING,
        .string = string
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_bool_skip(size_t condition, bool expect, bool result) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_BOOL_SKIP,
        .bool_skip = {
            .condition = condition,
            .expect = expect,
            .result = result 
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_cast_int(size_t step_id, HirTypeId source, HirTypeId dst) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_CAST_INT,
        .cast_int = {
            .step_id = step_id,
            .source = source,
            .dest = dst
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_deref(size_t step_id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_DEREF,
        .deref_step = step_id
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_take_ref(size_t step_id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_TAKE_REF,
        .ref_step = step_id
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_call(size_t callable, size_t *args) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_CALL,
        .call = {
            .args = args,
            .callable = callable
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_get_decl(HirDeclId id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_GET_DECL,
        .decl_id = id
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_get_local(HirLocalId id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_GET_LOCAL,
        .local_id = id
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_struct_field(size_t idx, size_t step_id) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_STRUCT_FIELD,
        .struct_field = {
            .idx = idx,
            .step = step_id,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_binop(HirBinop binop) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_BINOP,
        .binop = binop
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_const(HirConst constant) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_CONST,
        .constant = constant 
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_build_struct(HirTypeId type_id, size_t *fields) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_BUILD_STRUCT,
        .build_struct = {
            .type = type_id,
            .fields = fields
        }
    };
    return step;
}

typedef struct HirExpr {
    HirExprStep *steps;
} HirExpr;

static inline HirExpr hir_expr_new(HirExprStep *steps) {
    HirExpr expr = { .steps = steps };
    return expr;
}
