#pragma once

#include "ast/api/path.h"
#include "ast/api/expr.h"
#include "core/mempool.h"
#include <stdint.h>

typedef enum {
    AST_EXPR_PATH,
    AST_EXPR_INTEGER,
} AstExprKind;

typedef struct AstExpr {
    AstExprKind kind;

    union {
        AstPath *path;
        uint64_t integer;
    };
} AstExpr;

bool ast_expr_eq(const AstExpr *a, const AstExpr *b);

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path);
AstExpr *ast_expr_new_integer(Mempool *mempool, uint64_t integer);
