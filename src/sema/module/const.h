#pragma once

#include "sema/module/api/type.h"

typedef enum {
    SEMA_CONST_INT
} SemaConstKind;

typedef struct SemaConst {
    SemaConstKind kind;
    SemaType *type;

    union {
        uint64_t integer;
    };
} SemaConst;

SemaConst *sema_const_new_integer(Mempool *mempool, SemaType *type, uint64_t integer);
