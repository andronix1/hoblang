#pragma once

#include "ast/api/generic.h"
#include "ast/api/path.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_PATH_SEGMENT_IDENT,
    AST_PATH_SEGMENT_GENERIC_BUILD,
    AST_PATH_SEGMENT_DEREF,
    AST_PATH_SEGMENT_SIZEOF,
    AST_PATH_SEGMENT_TYPEOF,
} AstPathSegmentKind;

typedef struct AstPathSegment {
    AstPathSegmentKind kind;
    Slice slice;

    union {
        Slice ident;
        AstGenericBuilder *generic;
    };
} AstPathSegment;

typedef struct AstPath {
    AstPathSegment *segments;
} AstPath;

bool ast_path_eq(const AstPath *a, const AstPath *b);

AstPathSegment ast_path_segment_new_typeof(Slice slice);
AstPathSegment ast_path_segment_new_ident(Slice ident);
AstPathSegment ast_path_segment_new_deref(Slice slice);
AstPathSegment ast_path_segment_new_sizeof(Slice slice);
AstPathSegment ast_path_segment_new_generic_build(Slice slice, AstGenericBuilder *builder);
AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments);

Slice ast_path_slice(AstPath *path);
