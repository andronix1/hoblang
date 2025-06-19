#pragma once

#include "core/mempool.h"
#include "hir/api/decl.h"
#include "hir/api/func.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include <stdarg.h>

typedef enum {
    SEMA_VALUE_TYPE,
    SEMA_VALUE_MODULE,
    SEMA_VALUE_RUNTIME,
} SemaValueKind;

typedef struct SemaValue {
    SemaValueKind kind;

    union {
        SemaType *type;
        SemaModule *module;
        SemaValueRuntime runtime;
    };
} SemaValue;

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type);
SemaValue *sema_value_new_module(Mempool *mempool, SemaModule *module);
SemaValue *sema_value_new_runtime_global(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, HirDeclId id);
SemaValue *sema_value_new_runtime_local(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, HirLocalId id);
SemaValue *sema_value_new_runtime_expr_step(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, size_t id);
SemaValue *sema_value_new_runtime_const(Mempool *mempool, SemaConst *constant);
SemaValue *sema_value_new_runtime_ext_expr_step(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, size_t step_id, size_t of);
void sema_value_print(va_list list);
