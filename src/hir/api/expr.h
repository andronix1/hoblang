#pragma once

#include "hir/api/const.h"
#include "hir/api/decl.h"
#include "hir/api/expr/binop.h"
#include "hir/api/func.h"
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
    HIR_EXPR_STEP_SIZEOF,
    HIR_EXPR_STEP_NEG,
    HIR_EXPR_STEP_PTR_TO_INT,
    HIR_EXPR_STEP_INT_TO_PTR,
    HIR_EXPR_STEP_CAST_PTR,
} HirExprStepKind;

typedef struct {
    HirExprStepKind kind;

    HirType *type;

    union {
        HirConst constant;

        HirBinop binop;

        struct {
            size_t idx;
            size_t step;
        } struct_field;

        struct {
            size_t *elements;
            HirType *type;
        } build_array;

        HirLocalId local_id;
        HirDeclId decl_id;

        struct {
            size_t value;
            size_t idx;
        } idx_pointer;

        struct {
            size_t value;
            size_t idx;
        } idx_array;

        struct {
            HirNumberInfo info;
            size_t step;
        } neg;

        struct {
            HirType *of;
            HirType *type;
        } size;

        struct {
            size_t callable;
            size_t *args;
        } call;

        struct {
            size_t step_id;
            HirType *type;
        } cast_ptr;

        struct {
            size_t step_id;
            HirType *type;
        } int_to_ptr;

        struct {
            size_t step_id;
            HirType *type;
        } ptr_to_int;

        struct {
            size_t step_id;
            HirType *source, *dest;
        } cast_int;

        struct {
            size_t condition;
            bool expect;
            bool result;
        } bool_skip;

        struct {
            HirType *type;
            size_t *fields;
        } build_struct;

        size_t ref_step;
        size_t deref_step;
        size_t not_step;
    };
} HirExprStep;

static inline HirExprStep hir_expr_step_new_ptr_to_int(size_t step_id, HirType *dest) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_PTR_TO_INT,
        .ptr_to_int = {
            .type = dest,
            .step_id = step_id,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_int_to_ptr(size_t step_id, HirType *dest) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_INT_TO_PTR,
        .int_to_ptr = {
            .type = dest,
            .step_id = step_id,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_sizeof(HirType *of, HirType *type) {
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

static inline HirExprStep hir_expr_step_new_array(HirType *type, size_t *elements) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_BUILD_ARRAY,
        .build_array = {
            .elements = elements,
            .type = type
        }
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

static inline HirExprStep hir_expr_step_new_cast_ptr(size_t step_id, HirType *type) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_CAST_PTR,
        .cast_ptr = {
            .step_id = step_id,
            .type = type,
        }
    };
    return step;
}

static inline HirExprStep hir_expr_step_new_cast_int(size_t step_id, HirType *source, HirType *dest) {
    HirExprStep step = {
        .kind = HIR_EXPR_STEP_CAST_INT,
        .cast_int = {
            .step_id = step_id,
            .source = source,
            .dest = dest,
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

static inline HirExprStep hir_expr_step_new_build_struct(HirType *type_id, size_t *fields) {
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
