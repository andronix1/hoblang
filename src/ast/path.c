#include "path.h"
#include "ast/api/path.h"
#include "ast/mempool.h"

AstPathSegment ast_path_segment_new_ident(Slice ident) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_IDENT,
        .ident = ident
    };
    return result;
}
AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments)
    MEMPOOL_CONSTRUCT(AstPath, out->segments = segments)
