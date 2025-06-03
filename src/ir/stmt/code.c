#include "code.h"
#include "core/mempool.h"

IrCode *ir_code_new(Mempool *mempool, IrStmt **stmts)
    MEMPOOL_CONSTRUCT(IrCode,
        out->stmts = stmts;
    )
