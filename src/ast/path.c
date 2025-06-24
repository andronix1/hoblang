#include "path.h"
#include "ast/api/generic.h"
#include "ast/api/path.h"
#include "core/mempool.h"
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
            case AST_PATH_SEGMENT_GENERIC_BUILD:
                if (!ast_generic_builder_eq(as->generic, bs->generic)) return false;
                break;
            case AST_PATH_SEGMENT_SIZEOF: break;
            case AST_PATH_SEGMENT_DEREF: break;
            case AST_PATH_SEGMENT_TYPEOF: break;
        }
    }
    return true;
}


AstPathSegment ast_path_segment_new_generic_build(Slice slice, AstGenericBuilder *builder) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_GENERIC_BUILD,
        .slice = slice,
        .generic = builder,
    };
    return result;
}

AstPathSegment ast_path_segment_new_typeof(Slice slice) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_TYPEOF,
        .slice = slice,
    };
    return result;
}

AstPathSegment ast_path_segment_new_sizeof(Slice slice) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_SIZEOF,
        .slice = slice,
    };
    return result;
}

AstPathSegment ast_path_segment_new_deref(Slice slice) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_DEREF,
        .slice = slice,
    };
    return result;
}

AstPathSegment ast_path_segment_new_ident(Slice ident) {
    AstPathSegment result = {
        .kind = AST_PATH_SEGMENT_IDENT,
        .slice = ident,
        .ident = ident
    };
    return result;
}

AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments)
    MEMPOOL_CONSTRUCT(AstPath,
        assert(vec_len(segments) > 0);
        out->segments = segments;
    )

Slice ast_path_slice(AstPath *path) {
    assert(vec_len(path->segments) > 0);
    Slice slice = path->segments->slice;
    for (size_t i = 1; i < vec_len(path->segments); i++) {
        slice = slice_union(slice, path->segments[i].slice);
    }
    return slice;
}
