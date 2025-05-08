#pragma once

#include "ast/api/path.h"
#include "core/mempool.h"

typedef enum {
    AST_EXPR_PATH,
} AstExprKind;

typedef struct AstExpr {
    AstExprKind kind;

    union {
        AstPath *path;
    };
} AstExpr;

bool ast_expr_eq(const AstExpr *a, const AstExpr *b);

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path);
