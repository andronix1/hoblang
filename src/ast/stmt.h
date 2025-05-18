#pragma once

#include "ast/api/expr.h"
#include "ast/api/body.h"
#include "ast/expr.h"
#include "core/mempool.h"

typedef enum {
    AST_STMT_EXPR,
    AST_STMT_ASSIGN,
    AST_STMT_RETURN,
    AST_STMT_IF,
} AstStmtKind;

typedef struct {
    AstExpr *cond;
    AstBody *body;
} AstCondBlock;

typedef struct AstIf {
    AstCondBlock *conds;
    AstBody *else_body;

    struct {
        bool breaks;
    } sema;
} AstIf;

typedef struct {
    AstExpr *dst, *what;
    struct {
        bool is;
        AstBinopKind kind;
    } short_assign;
} AstAssign;

typedef struct {
    Slice slice;
    AstExpr *value;
} AstReturn;

typedef struct AstStmt {
    AstStmtKind kind;

    union {
        AstExpr *expr;
        AstReturn ret;
        AstAssign assign;
        AstIf if_else;
    };
} AstStmt;

bool ast_stmt_eq(const AstStmt *a, const AstStmt *b);

static inline AstCondBlock ast_cond_block_new(AstExpr *expr, AstBody *body) {
    AstCondBlock block = {
        .cond = expr,
        .body = body
    };
    return block;
}

AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr);
AstStmt *ast_stmt_new_assign(Mempool *mempool, AstExpr *dst, AstExpr *what);
AstStmt *ast_stmt_new_short_assign(Mempool *mempool, AstExpr *dst, AstExpr *what, AstBinopKind binop);
AstStmt *ast_stmt_new_return(Mempool *mempool, Slice slice, AstExpr *value);
AstStmt *ast_stmt_new_if(Mempool *mempool, AstCondBlock *conds, AstBody *else_body);
