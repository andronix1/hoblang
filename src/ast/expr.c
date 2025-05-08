#include "expr.h"
#include "ast/mempool.h"
#include "ast/path.h"
#include "core/assert.h"

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
        case AST_EXPR_INTEGER: return a->integer == b->integer;
    }
    UNREACHABLE;
}

AstExpr *ast_expr_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_EXPR_PATH, out->path = path)

AstExpr *ast_expr_new_integer(Mempool *mempool, uint64_t integer)
    CONSTRUCT(AST_EXPR_INTEGER, out->integer = integer)
