#pragma once

#include "core/mempool.h"
#include "ir/api/decl.h"
#include "ir/api/local.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include <stdarg.h>

typedef enum {
    SEMA_VALUE_TYPE,
    SEMA_VALUE_RUNTIME,
} SemaValueKind;

typedef struct SemaValue {
    SemaValueKind kind;

    union {
        SemaType *type;
        SemaValueRuntime runtime;
    };
} SemaValue;

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_runtime_global(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, IrDeclId id);
SemaValue *sema_value_new_runtime_local(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, IrLocalId id);
SemaValue *sema_value_new_runtime_expr_step(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, size_t id);
void sema_value_print(va_list list);
