#include "stmt.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "ast/type.h"
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
        case AST_STMT_FOR:
            return
                ast_expr_eq(a->for_loop.iterator, b->for_loop.iterator) &&
                slice_eq(a->for_loop.iter, b->for_loop.iter) &&
                opt_slice_eq(&a->for_loop.label, &b->for_loop.label) &&
                ast_type_eq(a->for_loop.type, b->for_loop.type) &&
                ast_body_eq(a->for_loop.body, b->for_loop.body);
            return equals_nullable(a->ret.value, b->ret.value, (EqFunc)ast_expr_eq);
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
                opt_slice_eq(&a->while_loop.label, &b->while_loop.label) &&
                ast_body_eq(a->while_loop.body, b->while_loop.body);
        case AST_STMT_CONTINUE:
            return loop_controls_eq(&a->continue_loop, &b->continue_loop);
        case AST_STMT_BREAK:
            return loop_controls_eq(&a->break_loop, &b->break_loop);
        case AST_STMT_DEFER:
            if (a->defer.kind != b->defer.kind) {
                return false;
            }
            switch (a->defer.kind) {
                case AST_DEFER_BODY: return ast_body_eq(a->defer.body, b->defer.body);
                case AST_DEFER_EXPR: return ast_expr_eq(a->defer.expr, b->defer.expr);
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}

AstStmt *ast_stmt_new_continue(Mempool *mempool, AstLoopControl control)
    CONSTRUCT(AST_STMT_CONTINUE, out->continue_loop = control;)

AstStmt *ast_stmt_new_break(Mempool *mempool, AstLoopControl control)
    CONSTRUCT(AST_STMT_BREAK, out->break_loop = control;)

AstStmt *ast_stmt_new_for(Mempool *mempool, Slice iter_name, AstExpr *iterator, AstBody *body, OptSlice label)
    CONSTRUCT(AST_STMT_FOR,
        out->for_loop.iter = iter_name;
        out->for_loop.body = body;
        out->for_loop.label = label;
        out->for_loop.iterator = iterator;
    )

AstStmt *ast_stmt_new_defer_expr(Mempool *mempool, AstExpr *expr)
    CONSTRUCT(AST_STMT_DEFER,
        out->defer.kind = AST_DEFER_EXPR;
        out->defer.expr = expr;
    )

AstStmt *ast_stmt_new_defer_body(Mempool *mempool, AstBody *body)
    CONSTRUCT(AST_STMT_DEFER,
        out->defer.kind = AST_DEFER_BODY;
        out->defer.body = body;
    )

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

AstStmt *ast_stmt_new_while(Mempool *mempool, AstExpr *expr, AstBody *body, bool is_do_while, OptSlice label)
    CONSTRUCT(AST_STMT_WHILE,
        out->while_loop.cond = expr;
        out->while_loop.body = body;
        out->while_loop.is_do_while = is_do_while;
        out->while_loop.label = label;
    )

AstStmt *ast_stmt_new_if(Mempool *mempool, AstCondBlock *conds, AstBody *else_body)
    CONSTRUCT(AST_STMT_IF, out->if_else.conds = conds; out->if_else.else_body = else_body;)
