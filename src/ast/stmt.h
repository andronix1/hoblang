#pragma once

#include "ast/api/expr.h"
#include "ast/api/body.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "core/mempool.h"

typedef enum {
    AST_STMT_EXPR,
    AST_STMT_ASSIGN,
    AST_STMT_RETURN,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_CONTINUE,
    AST_STMT_BREAK,
    AST_STMT_DEFER,
    AST_STMT_FOR,
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
    AstExpr *cond;
    AstBody *body;
    bool is_do_while;
    OptSlice label;
} AstWhile;

typedef struct {
    Slice slice;
    AstExpr *value;
} AstReturn;

typedef struct {
    Slice iter;
    AstType *type;
    AstExpr *iterator;
    AstBody *body;
    OptSlice label;
} AstFor;

typedef enum {
    AST_DEFER_BODY,
    AST_DEFER_EXPR,
} AstDeferKind;

typedef struct {
    AstDeferKind kind;

    union {
        AstExpr *expr;
        AstBody *body;
    };
} AstDefer;

typedef struct {
    bool is_labeled;
    Slice label;
} AstLoopControl;

static inline AstLoopControl ast_loop_control_new() {
    AstLoopControl control = {
        .is_labeled = false
    };
    return control;
}

static inline AstLoopControl ast_loop_control_new_labeled(Slice label) {
    AstLoopControl control = {
        .is_labeled = true,
        .label = label
    };
    return control;
}

typedef struct AstStmt {
    AstStmtKind kind;

    union {
        AstExpr *expr;
        AstDefer defer;
        AstReturn ret;
        AstAssign assign;
        AstIf if_else;
        AstWhile while_loop;
        AstFor for_loop;
        AstLoopControl break_loop;
        AstLoopControl continue_loop;
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

AstStmt *ast_stmt_new_continue(Mempool *mempool, AstLoopControl control);
AstStmt *ast_stmt_new_break(Mempool *mempool, AstLoopControl control);
AstStmt *ast_stmt_new_defer_body(Mempool *mempool, AstBody *body);
AstStmt *ast_stmt_new_defer_expr(Mempool *mempool, AstExpr *expr);
AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr);
AstStmt *ast_stmt_new_assign(Mempool *mempool, AstExpr *dst, AstExpr *what);
AstStmt *ast_stmt_new_short_assign(Mempool *mempool, AstExpr *dst, AstExpr *what, AstBinopKind binop);
AstStmt *ast_stmt_new_return(Mempool *mempool, Slice slice, AstExpr *value);
AstStmt *ast_stmt_new_if(Mempool *mempool, AstCondBlock *conds, AstBody *else_body);
AstStmt *ast_stmt_new_while(Mempool *mempool, AstExpr *expr, AstBody *body, bool is_do_while, OptSlice label);
AstStmt *ast_stmt_new_for(Mempool *mempool, Slice iter_name, AstExpr *iterator, AstBody *body, OptSlice label);
