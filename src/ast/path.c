#include "path.h"
#include "ast/api/path.h"
#include "ast/mempool.h"

AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments)
    MEMPOOL_CONSTRUCT(AstPath, out->segments = segments)
