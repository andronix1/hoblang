#pragma once

#include "stmt.h"
#include "ast/expr.h"
#include "ast/mempool.h"
#include "core/assert.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstStmt, { \
    out->kind = KIND; \
    FIELDS; \
})

bool ast_stmt_eq(const AstStmt *a, const AstStmt *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_STMT_EXPR: return ast_expr_eq(a->expr, b->expr);
    }
    UNREACHABLE;
}

AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr)
    CONSTRUCT(AST_STMT_EXPR, out->expr = expr)
