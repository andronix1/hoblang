#pragma once

#include "ast/api/path.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_PATH_SEGMENT_IDENT,
} AstPathSegmentKind;

typedef struct {
    AstPathSegmentKind kind;

    union {
        Slice ident;
    };
} AstPathSegment;

typedef struct AstPath {
    AstPathSegment *segments;
} AstPath;

AstPath *ast_path_new(Mempool *mempool, AstPathSegment *segments);
