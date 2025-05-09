#pragma once

#include "ast/api/path.h"
#include "ast/api/expr.h"
#include "core/mempool.h"
#include <stdint.h>

typedef enum {
    AST_EXPR_PATH,
    AST_EXPR_INTEGER,
    AST_EXPR_CALL,
    AST_EXPR_SCOPE,
    AST_EXPR_BINOP,
} AstExprKind;

typedef struct {
    AstExpr *inner;
    AstExpr **args;
} AstCall;

typedef enum {
    AST_BINOP_ADD, AST_BINOP_SUBTRACT,
    AST_BINOP_MULTIPLY, AST_BINOP_DIVIDE,
} AstBinopKind;

typedef struct {
    AstBinopKind kind;
    AstExpr *left, *right;
} AstBinop;

typedef struct AstExpr {
    AstExprKind kind;

    union {
        AstPath *path;
        AstExpr *scope;
        uint64_t integer;
        AstCall call;
        AstBinop binop;
    };
} AstExpr;

bool ast_expr_eq(const AstExpr *a, const AstExpr *b);

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path);
AstExpr *ast_expr_new_integer(Mempool *mempool, uint64_t integer);
AstExpr *ast_expr_new_callable(Mempool *mempool, AstExpr *inner, AstExpr **args);
AstExpr *ast_expr_new_scope(Mempool *mempool, AstExpr *inner);
AstExpr *ast_expr_new_binop(Mempool *mempool, AstBinopKind kind, AstExpr *left, AstExpr *right);
