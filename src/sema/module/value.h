#pragma once

#include "core/mempool.h"
#include "sema/module/api.h"
#include "sema/module/api/decl_handle.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/type.h"

typedef enum {
    SEMA_VALUE_RUNTIME_VAR,
    SEMA_VALUE_RUNTIME_FINAL,
} SemaValueRuntimeKind;

typedef struct {
    SemaValueRuntimeKind kind;
    SemaType *type;
    SemaDeclHandle *ext;
} SemaValueRuntime;

typedef enum {
    SEMA_VALUE_RUNTIME,
    SEMA_VALUE_TYPE,
    SEMA_VALUE_GENERIC,
    SEMA_VALUE_MODULE,
} SemaValueKind;

typedef struct SemaValue {
    SemaValueKind kind;

    union {
        SemaValueRuntime runtime;
        SemaType *type;
        SemaGeneric *generic;
        SemaModule *module;
    };
} SemaValue;

SemaValue *sema_value_new_nested(Mempool *mempool, SemaValue *base, SemaType *type);
SemaValue *sema_value_new_ext(Mempool *mempool, SemaValue *base, SemaDeclHandle *handle);

SemaValue *sema_value_new_var(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_final(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_module(Mempool *mempool, SemaModule *module);
SemaValue *sema_value_new_generic(Mempool *mempool, SemaGeneric *generic);
