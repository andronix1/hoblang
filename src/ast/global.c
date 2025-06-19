#include "global.h"
#include "core/mempool.h"

AstGlobal *ast_global_new(Mempool *mempool, OptSlice alias)
    MEMPOOL_CONSTRUCT(AstGlobal, out->alias = alias)
