#include "value.h"
#include "core/mempool.h"
#include <stdio.h>

void sema_value_print(va_list list) {
    SemaValue *value = va_arg(list, SemaValue*);
    switch (value->kind) {
        case SEMA_VALUE_TYPE: printf("type"); break;
        case SEMA_VALUE_RUNTIME: printf("value"); break;
    }
}

SemaValue *sema_value_new_runtime_global(Mempool *mempool, SemaType *type, IrDeclId id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.kind = SEMA_VALUE_RUNTIME_GLOBAL;
        out->runtime.global_id = id;
    );

SemaValue *sema_value_new_runtime_local(Mempool *mempool, SemaType *type, IrLocalId id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.kind = SEMA_VALUE_RUNTIME_LOCAL;
        out->runtime.local_id = id;
    );

SemaValue *sema_value_new_runtime_expr_step(Mempool *mempool, SemaType *type, size_t id)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_RUNTIME;
        out->runtime.type = type;
        out->runtime.kind = SEMA_VALUE_RUNTIME_EXPR_STEP;
        out->runtime.step_id = id;
    );

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type)
    MEMPOOL_CONSTRUCT(SemaValue,
        out->kind = SEMA_VALUE_TYPE;
        out->type = type
    );
