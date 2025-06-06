#include "decl.h"
#include "core/mempool.h"
#include "sema/module/value.h"

SemaDecl *sema_decl_new(Mempool *mempool, SemaValue *value)
    MEMPOOL_CONSTRUCT(SemaDecl,
        out->value = value;
    )
