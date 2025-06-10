#include "stmt.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "core/mempool.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/slice.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstStmt, { \
    out->kind = KIND; \
    FIELDS; \
})

static inline bool loop_controls_eq(const AstLoopControl *a, const AstLoopControl *b) {
    if (a->is_labeled != b->is_labeled) {
        return false;
    }
    if (a->is_labeled) {
        return slice_eq(a->label, b->label);
    }
    return true;
}

bool ast_stmt_eq(const AstStmt *a, const AstStmt *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_STMT_EXPR: return ast_expr_eq(a->expr, b->expr);
        case AST_STMT_RETURN: return equals_nullable(a->ret.value, b->ret.value, (EqFunc)ast_expr_eq);
        case AST_STMT_ASSIGN: return
            a->assign.short_assign.is == b->assign.short_assign.is &&
            (
                !a->assign.short_assign.is ||
                a->assign.short_assign.kind.kind == b->assign.short_assign.kind.kind
            ) &&
            ast_expr_eq(a->assign.dst, b->assign.dst) &&
            ast_expr_eq(a->assign.what, b->assign.what);
        case AST_STMT_IF:
            if (vec_len(a->if_else.conds) != vec_len(b->if_else.conds)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(b->if_else.conds); i++) {
                AstCondBlock *ab = &a->if_else.conds[i];
                AstCondBlock *bb = &b->if_else.conds[i];
                if (!ast_expr_eq(ab->cond, bb->cond) || !ast_body_eq(ab->body, bb->body)) {
                    return false;
                }
            }
            return equals_nullable(a->if_else.else_body, b->if_else.else_body, (EqFunc)ast_body_eq);
        case AST_STMT_WHILE:
            return ast_expr_eq(a->while_loop.cond, b->while_loop.cond) &&
                ast_body_eq(a->while_loop.body, b->while_loop.body);
        case AST_STMT_CONTINUE:
            return loop_controls_eq(&a->continue_loop, &b->continue_loop);
        case AST_STMT_BREAK:
            return loop_controls_eq(&a->break_loop, &b->break_loop);
    }
    UNREACHABLE;
}

AstStmt *ast_stmt_new_continue(Mempool *mempool, AstLoopControl control)
    CONSTRUCT(AST_STMT_CONTINUE, out->continue_loop = control;)

AstStmt *ast_stmt_new_break(Mempool *mempool, AstLoopControl control)
    CONSTRUCT(AST_STMT_BREAK, out->break_loop = control;)

AstStmt *ast_stmt_new_assign(Mempool *mempool, AstExpr *dst, AstExpr *what)
    CONSTRUCT(AST_STMT_ASSIGN,
        out->assign.dst = dst;
        out->assign.what = what;
        out->assign.short_assign.is = false;
    )

AstStmt *ast_stmt_new_short_assign(Mempool *mempool, AstExpr *dst, AstExpr *what, AstBinopKind binop)
    CONSTRUCT(AST_STMT_ASSIGN,
        out->assign.dst = dst;
        out->assign.what = what;
        out->assign.short_assign.is = true;
        out->assign.short_assign.kind = binop;
    )

AstStmt *ast_stmt_new_expr(Mempool *mempool, AstExpr *expr)
    CONSTRUCT(AST_STMT_EXPR, out->expr = expr)

AstStmt *ast_stmt_new_return(Mempool *mempool, Slice slice, AstExpr *value)
    CONSTRUCT(AST_STMT_RETURN, out->ret.value = value; out->ret.slice = slice;)

AstStmt *ast_stmt_new_while(Mempool *mempool, AstExpr *expr, AstBody *body, bool is_do_while)
    CONSTRUCT(AST_STMT_WHILE,
        out->while_loop.cond = expr;
        out->while_loop.body = body;
        out->while_loop.is_do_while = is_do_while;
        out->while_loop.label.has = false;
    )

AstStmt *ast_stmt_new_while_labeled(Mempool *mempool, AstExpr *expr, AstBody *body, bool is_do_while, Slice label)
    CONSTRUCT(AST_STMT_WHILE,
        out->while_loop.cond = expr;
        out->while_loop.body = body;
        out->while_loop.is_do_while = is_do_while;
        out->while_loop.label.has = true;
        out->while_loop.label.name = label;
    )

AstStmt *ast_stmt_new_if(Mempool *mempool, AstCondBlock *conds, AstBody *else_body)
    CONSTRUCT(AST_STMT_IF, out->if_else.conds = conds; out->if_else.else_body = else_body;)
