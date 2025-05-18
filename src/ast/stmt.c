#include "stmt.h"
#include "ast/body.h"
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
    }
    UNREACHABLE;
}

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

AstStmt *ast_stmt_new_if(Mempool *mempool, AstCondBlock *conds, AstBody *else_body)
    CONSTRUCT(AST_STMT_IF, out->if_else.conds = conds; out->if_else.else_body = else_body;)
