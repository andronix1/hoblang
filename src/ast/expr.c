#include "expr.h"
#include "ast/mempool.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/vec.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstExpr, { \
    out->kind = KIND; \
    FIELDS; \
})

bool ast_expr_eq(const AstExpr *a, const AstExpr *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_EXPR_PATH: return ast_path_eq(a->path, b->path);
        case AST_EXPR_CALL:
            if (!ast_expr_eq(a->call.inner, b->call.inner) ||
                vec_len(a->call.args) != vec_len(b->call.args)
            ) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->call.args); i++) {
                if (!ast_expr_eq(a->call.args[i], b->call.args[i])) {
                    return false;
                }
            }
            return true;
        case AST_EXPR_INTEGER: return a->integer == b->integer;
        case AST_EXPR_SCOPE: return ast_expr_eq(a->scope, b->scope);
        case AST_EXPR_BINOP: return
            a->binop.kind == b->binop.kind &&
            ast_expr_eq(a->binop.left, b->binop.left) &&
            ast_expr_eq(a->binop.right, b->binop.right);
    }
    UNREACHABLE;
}

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_EXPR_PATH, out->path = path)

AstExpr *ast_expr_new_integer(Mempool *mempool, uint64_t integer)
    CONSTRUCT(AST_EXPR_INTEGER, out->integer = integer)

AstExpr *ast_expr_new_scope(Mempool *mempool, AstExpr *inner)
    CONSTRUCT(AST_EXPR_SCOPE, out->scope = inner)

AstExpr *ast_expr_new_callable(Mempool *mempool, AstExpr *inner, AstExpr **args)
    CONSTRUCT(AST_EXPR_CALL,
        out->call.inner = inner;
        out->call.args = args;
    )

AstExpr *ast_expr_new_binop(Mempool *mempool, AstBinopKind kind, AstExpr *left, AstExpr *right)
    CONSTRUCT(AST_EXPR_BINOP,
        out->binop.kind = kind;
        out->binop.left = left;
        out->binop.right = right;
    )
