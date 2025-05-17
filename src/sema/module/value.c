#include "value.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/value.h"
#include <string.h>

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(SemaValue, { out->kind = KIND; out->runtime.ext = NULL; FIELDS })

SemaValue *sema_value_new_ext(Mempool *mempool, SemaValue *base, SemaDeclHandle *handle) {
    SemaValue *new = mempool_alloc(mempool, SemaValue);
    memcpy(new, base, sizeof(SemaValue));
    new->runtime.ext = handle;
    return new;
}

SemaValue *sema_value_new_nested(Mempool *mempool, SemaValue *base, SemaType *type) {
    assert(sema_value_is_runtime(base));
    switch (base->runtime.kind) {
        case SEMA_VALUE_RUNTIME_VAR: return sema_value_new_var(mempool, type);
        case SEMA_VALUE_RUNTIME_FINAL: return sema_value_new_final(mempool, type);
    }
    UNREACHABLE;
}

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

SemaValue *sema_value_new_generic(Mempool *mempool, SemaGeneric *generic)
    CONSTRUCT(SEMA_VALUE_GENERIC,
        out->generic = generic;
    )

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type)
    CONSTRUCT(SEMA_VALUE_TYPE,
        out->type = type;
    )
