#include "value.h"
#include "core/mempool.h"
#include "sema/module/api/value.h"
#include <stdio.h>
#include <string.h>

void sema_value_print(va_list list) {
    SemaValue *value = va_arg(list, SemaValue*);
    switch (value->kind) {
        case SEMA_VALUE_TYPE: printf("type"); break;
        case SEMA_VALUE_RUNTIME: printf("value"); break;
        case SEMA_VALUE_MODULE: printf("module"); break;
    }
}

SemaValue *sema_value_new_runtime_global(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, IrDeclId id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.val_kind = SEMA_VALUE_RUNTIME_GLOBAL;
        out->runtime.kind = kind;
        out->runtime.global_id = id;
    );

SemaValue *sema_value_new_runtime_local(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, IrLocalId id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.val_kind = SEMA_VALUE_RUNTIME_LOCAL;
        out->runtime.kind = kind;
        out->runtime.local_id = id;
    );

SemaValue *sema_value_new_runtime_expr_step(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, size_t id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.val_kind = SEMA_VALUE_RUNTIME_EXPR_STEP;
        out->runtime.kind = kind;
        out->runtime.in_expr_id.step_id = id;
        out->runtime.in_expr_id.is_ext = false;
    );

SemaValue *sema_value_new_runtime_ext_expr_step(Mempool *mempool, SemaRuntimeKind kind, SemaType *type, size_t step_id, size_t of)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.val_kind = SEMA_VALUE_RUNTIME_EXPR_STEP;
        out->runtime.kind = kind;
        out->runtime.in_expr_id.step_id = step_id;
        out->runtime.in_expr_id.is_ext = true;
        out->runtime.in_expr_id.ext_of = of;
    );

SemaValue *sema_value_new_module(Mempool *mempool, SemaModule *module)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_MODULE;
        out->module = module; 
    );

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_TYPE;
        out->type = type
    );
