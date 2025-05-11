#include "value.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(SemaValue, { out->kind = KIND; FIELDS })

SemaValue *sema_value_new_var(Mempool *mempool, SemaType *type)
    CONSTRUCT(SEMA_VALUE_RUNTIME,
        out->runtime.kind = SEMA_VALUE_RUNTIME_VAR;
        out->runtime.type = type;
    )

SemaValue *sema_value_new_final(Mempool *mempool, SemaType *type)
    CONSTRUCT(SEMA_VALUE_RUNTIME,
        out->runtime.kind = SEMA_VALUE_RUNTIME_FINAL;
        out->runtime.type = type;
    )

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type)
    CONSTRUCT(SEMA_VALUE_TYPE,
        out->type = type;
    )
