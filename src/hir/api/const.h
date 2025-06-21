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
    HIR_CONST_GEN_FUNC,
} HirConstKind;

typedef struct HirConst HirConst;

typedef struct HirConst {
    HirConstKind kind;

    HirTypeId type;

    union {
        bool boolean;
        uint64_t integer;
        HirConst **struct_fields;
        HirDeclId func_decl;

        struct {
            HirTypeId *params;
            HirGenScopeId scope;
            HirGenFuncId func;

            HirGenUsageId usage;
        } gen_func;
    };
    long double real; // Because of ABI warning :(
} HirConst;

static inline HirConst hir_const_new_gen_func(HirTypeId type, HirGenScopeId scope, HirGenFuncId func, HirTypeId *params) {
    HirConst constant = {
        .kind = HIR_CONST_GEN_FUNC,
        .type = type,
        .gen_func = {
            .scope = scope,
            .func = func,
            .params = params,
        }
    };
    return constant;
}

static inline HirConst hir_const_new_int(HirTypeId type, uint64_t value) {
    HirConst constant = {
        .kind = HIR_CONST_INT,
        .type = type,
        .integer = value,
    };
    return constant;
}

static inline HirConst hir_const_new_bool(HirTypeId type, bool value) {
    HirConst constant = {
        .kind = HIR_CONST_BOOL,
        .type = type,
        .boolean = value,
    };
    return constant;
}

static inline HirConst hir_const_new_real(HirTypeId type, long double value) {
    HirConst constant = {
        .kind = HIR_CONST_REAL,
        .type = type,
        .real = value,
    };
    return constant;
}

static inline HirConst hir_const_new_struct(HirTypeId type, HirConst **fields) {
    HirConst constant = {
        .kind = HIR_CONST_STRUCT,
        .type = type,
        .struct_fields = fields,
    };
    return constant;
}

static inline HirConst hir_const_new_func(HirTypeId type, HirDeclId decl) {
    HirConst constant = {
        .kind = HIR_CONST_FUNC,
        .type = type,
        .func_decl = decl,
    };
    return constant;
}
