#include "code.h"
#include "core/mempool.h"

HirCode *hir_code_new(Mempool *mempool, HirStmt *stmts)
    MEMPOOL_CONSTRUCT(HirCode,
        out->stmts = stmts;
        out->breaks = false;
    )
