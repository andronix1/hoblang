#pragma once

#include "ast/api/path.h"
#include "ast/api/expr.h"
#include "core/mempool.h"
#include <stdint.h>

typedef enum {
    AST_EXPR_PATH,
    AST_EXPR_INTEGER,
    AST_EXPR_CALL,
} AstExprKind;

typedef struct {
    AstExpr *inner;
    AstExpr **args;
} AstCall;

typedef struct AstExpr {
    AstExprKind kind;

    union {
        AstPath *path;
        uint64_t integer;
        AstCall call;
    };
} AstExpr;

bool ast_expr_eq(const AstExpr *a, const AstExpr *b);

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path);
AstExpr *ast_expr_new_integer(Mempool *mempool, uint64_t integer);
AstExpr *ast_expr_new_callable(Mempool *mempool, AstExpr *inner, AstExpr **args);
