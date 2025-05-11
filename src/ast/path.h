#pragma once

#include "ast/api/path.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_PATH_SEGMENT_IDENT,
} AstPathSegmentKind;

typedef struct {
    AstPathSegmentKind kind;
    Slice slice;

    union {
        Slice ident;
    };
} AstPathSegment;

typedef struct AstPath {
    AstPathSegment *segments;
} AstPath;

bool ast_path_eq(const AstPath *a, const AstPath *b);

AstPathSegment ast_path_segment_new_ident(Slice ident);
AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments);

Slice ast_path_slice(AstPath *path);
