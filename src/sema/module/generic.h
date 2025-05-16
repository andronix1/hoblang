#pragma once

#include "core/mempool.h"
#include "sema/module/type.h"

typedef struct {
    SemaType *type;
} SemaGenericParam;

typedef enum {
    SEMA_GENERIC_TYPE
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;
    SemaType *type;
    SemaGenericParam *params;
} SemaGeneric;

SemaGeneric *sema_generic_new(Mempool *mempool, SemaType *type, SemaGenericParam *params);

static inline SemaGenericParam sema_generic_param_new(SemaType *type) {
    SemaGenericParam param = {
        .type = type
    };
    return param;
}
