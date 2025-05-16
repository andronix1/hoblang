#pragma once

#include "core/mempool.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/type.h"

typedef enum {
    SEMA_VALUE_RUNTIME_VAR,
    SEMA_VALUE_RUNTIME_FINAL,
} SemaValueRuntimeKind;

typedef struct {
    SemaValueRuntimeKind kind;
    SemaType *type;
} SemaValueRuntime;

typedef enum {
    SEMA_VALUE_RUNTIME,
    SEMA_VALUE_TYPE,
    SEMA_VALUE_GENERIC,
} SemaValueKind;

typedef struct SemaValue {
    SemaValueKind kind;

    union {
        SemaValueRuntime runtime;
        SemaType *type;
        SemaGeneric *generic;
    };
} SemaValue;

SemaValue *sema_value_new_var(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_final(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_generic(Mempool *mempool, SemaGeneric *generic);
