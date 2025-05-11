#include "global.h"
#include "core/mempool.h"

AstGlobal *ast_global_new(Mempool *mempool)
    MEMPOOL_CONSTRUCT(AstGlobal, out->has_alias = false)

AstGlobal *ast_global_new_with_alias(Mempool *mempool, Slice alias)
    MEMPOOL_CONSTRUCT(AstGlobal,
        out->has_alias = true;
        out->alias = alias;
    )
