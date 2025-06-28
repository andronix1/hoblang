#pragma once

#include "hir/api/decl.h"
#include "hir/api/type.h"
#include <stdint.h>

typedef enum {
    HIR_CONST_BOOL,
    HIR_CONST_INT,
    HIR_CONST_REAL,
    HIR_CONST_STRUCT,
    HIR_CONST_FUNC,
    HIR_CONST_UNDEFINED,
    HIR_CONST_GEN_FUNC,
} HirConstKind;

typedef struct HirConst HirConst;

typedef struct HirConst {
    HirConstKind kind;

    HirType *type;

    union {
        bool boolean;
        uint64_t integer;
        HirConst **struct_fields;
        HirDeclId func_decl;
        struct {
            HirType **params;
            HirGenScopeId scope;
            HirGenFuncId func;
            bool is_from;
            HirGenScopeId from;

            HirGenUsageId usage;
        } gen_func;
    };
    long double real; // Because of ABI warning :(
} HirConst;

static inline HirConst hir_const_new_undefined(HirType *type) {
    HirConst constant = {
        .kind = HIR_CONST_UNDEFINED,
        .type = type
    };
    return constant;
}

static inline HirConst hir_const_new_gen_func(HirType *type, HirGenScopeId scope, HirGenFuncId func, HirType **params) {
    HirConst constant = {
        .kind = HIR_CONST_GEN_FUNC,
        .type = type,
        .gen_func = {
            .scope = scope,
            .func = func,
            .params = params,
            .is_from = false,
        }
    };
    return constant;
}

static inline HirConst hir_const_new_gen_func_from(HirType *type, HirGenScopeId scope, HirGenFuncId func, HirType **params, HirGenScopeId from) {
    HirConst constant = {
        .kind = HIR_CONST_GEN_FUNC,
        .type = type,
        .gen_func = {
            .scope = scope,
            .func = func,
            .params = params,
            .is_from = true,
            .from = from,
        }
    };
    return constant;
}

static inline HirConst hir_const_new_int(HirType *type, uint64_t value) {
    HirConst constant = {
        .kind = HIR_CONST_INT,
        .type = type,
        .integer = value,
    };
    return constant;
}

static inline HirConst hir_const_new_bool(HirType *type, bool value) {
    HirConst constant = {
        .kind = HIR_CONST_BOOL,
        .type = type,
        .boolean = value,
    };
    return constant;
}

static inline HirConst hir_const_new_real(HirType *type, long double value) {
    HirConst constant = {
        .kind = HIR_CONST_REAL,
        .type = type,
        .real = value,
    };
    return constant;
}

static inline HirConst hir_const_new_struct(HirType *type, HirConst **fields) {
    HirConst constant = {
        .kind = HIR_CONST_STRUCT,
        .type = type,
        .struct_fields = fields,
    };
    return constant;
}

static inline HirConst hir_const_new_func(HirType *type, HirDeclId decl) {
    HirConst constant = {
        .kind = HIR_CONST_FUNC,
        .type = type,
        .func_decl = decl,
    };
    return constant;
}
