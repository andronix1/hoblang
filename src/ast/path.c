#include "path.h"
#include "ast/api/path.h"
#include "ast/mempool.h"
#include "core/slice.h"
#include "core/vec.h"

bool ast_path_eq(const AstPath *a, const AstPath *b) {
    if (vec_len(a->segments) != vec_len(b->segments)) {
        return false;
    }
    for (size_t i = 0; i < vec_len(a->segments); i++) {
        AstPathSegment *as = &a->segments[i];
        AstPathSegment *bs = &b->segments[i];
        if (as->kind != bs->kind) {
            return false;
        }
        switch (as->kind) {
            case AST_PATH_SEGMENT_IDENT:
                if (!slice_eq(as->ident, bs->ident)) return false;
                break;
        }
    }
    return true;
}


AstPathSegment ast_path_segment_new_ident(Slice ident) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_IDENT,
        .ident = ident
    };
    return result;
}

AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments)
    MEMPOOL_CONSTRUCT(AstPath, out->segments = segments)
