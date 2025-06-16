#pragma once

#include "sema/module/api/type.h"

typedef enum {
    SEMA_CONST_INT,
    SEMA_CONST_FLOAT
} SemaConstKind;

typedef struct SemaConst {
    SemaConstKind kind;
    SemaType *type;

    union {
        uint64_t integer;
        long double float_value;
    };
} SemaConst;

SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer);
SemaConst *sema_const_new_float(Mempool *mempool, SemaType *type, long double value);
