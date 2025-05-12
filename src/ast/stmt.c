#include "stmt.h"
#include "ast/expr.h"
#include "core/mempool.h"
#include "core/assert.h"
#include "core/null.h"

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
        case AST_STMT_RETURN: return equals_nullable(a->ret.value, b->ret.value, (EqFunc)ast_expr_eq);
    }
    UNREACHABLE;
}

AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr)
    CONSTRUCT(AST_STMT_EXPR, out->expr = expr)

AstStmt *ast_stmt_new_return(Mempool *mempool, Slice slice, AstExpr *value)
    CONSTRUCT(AST_STMT_RETURN, out->ret.value = value; out->ret.slice = slice;)
