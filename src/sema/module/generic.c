#include "generic.h"
#include "core/mempool.h"

SemaGeneric *sema_generic_new(Mempool *mempool, SemaType *type, SemaGenericParam *params)
    MEMPOOL_CONSTRUCT(SemaGeneric,
        out->kind = SEMA_GENERIC_TYPE;
        out->type = type;
        out->params = params;
    )
