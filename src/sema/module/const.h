#pragma once

#include "sema/module/api/type.h"

typedef enum {
    SEMA_CONST_INT,
    SEMA_CONST_FLOAT,
    SEMA_CONST_FUNC,
    SEMA_CONST_GEN_FUNC,
    SEMA_CONST_UNDEFINED,
} SemaConstKind;

typedef struct SemaConst {
    SemaConstKind kind;
    SemaType *type;

    union {
        uint64_t integer;
        long double float_value;
        HirDeclId func_decl;
        struct {
            HirTypeId *params;
            HirGenScopeId scope;
            HirGenFuncId func;
        } gen_func;
    };
} SemaConst;

SemaConst *sema_const_new_undefined(Mempool *mempool, SemaType *type);
SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer);
SemaConst *sema_const_new_float(Mempool *mempool, SemaType *type, long double value);
SemaConst *sema_const_new_func(Mempool *mempool, SemaType *type, HirDeclId decl);
SemaConst *sema_const_new_gen_func(Mempool *mempool, SemaType *type, HirGenScopeId scope, HirGenFuncId func, HirTypeId *params);
