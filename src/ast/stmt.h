#pragma once

#include "ast/api/expr.h"
#include "core/mempool.h"

typedef enum {
    AST_STMT_EXPR
} AstStmtKind;

typedef struct AstStmt {
    AstStmtKind kind;

    union {
        AstExpr *expr;
    };
} AstStmt;

bool ast_stmt_eq(const AstStmt *a, const AstStmt *b);

AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr);
