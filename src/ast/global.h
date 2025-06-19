#pragma once

#include "core/mempool.h"
#include "core/opt_slice.h"

typedef struct AstGlobal {
    OptSlice alias;
} AstGlobal;

AstGlobal *ast_global_new(Mempool *mempool, OptSlice alias);
