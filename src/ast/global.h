#pragma once

#include "core/mempool.h"
#include "core/slice.h"

typedef struct AstGlobal {
    bool has_alias;
    Slice alias;
} AstGlobal;

AstGlobal *ast_global_new(Mempool *mempool);
AstGlobal *ast_global_new_with_alias(Mempool *mempool, Slice alias);
