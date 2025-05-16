#include "expr.h"
#include "ast/type.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/slice.h"
#include "core/vec.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstExpr, { \
    out->kind = KIND; \
    out->slice = slice; \
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
                vec_len(a->call.args) != vec_len(b->call.args)) return false;
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
        case AST_EXPR_STRING: return slice_eq(a->string, b->string);
        case AST_EXPR_STRUCT:
            if (!ast_type_eq(a->structure.type, b->structure.type)) return false;
            if (vec_len(a->structure.fields_map) != vec_len(b->structure.fields_map)) return false;
            for (size_t i = 0; i < vec_len(a->structure.fields_map); i++) {
                keymap_at(a->structure.fields_map, i, af);
                keymap_at(b->structure.fields_map, i, bf);
                if (!slice_eq(af->key, bf->key) || !ast_expr_eq(af->value.expr, bf->value.expr)) return false;
            }
            return true;
    }
    UNREACHABLE;
}

AstExpr *ast_expr_new_path(Mempool *mempool, Slice slice, AstPath *path)
    CONSTRUCT(AST_EXPR_PATH, out->path = path)

AstExpr *ast_expr_new_integer(Mempool *mempool, Slice slice, uint64_t integer)
    CONSTRUCT(AST_EXPR_INTEGER, out->integer = integer)

AstExpr *ast_expr_new_string(Mempool *mempool, Slice slice, Slice string)
    CONSTRUCT(AST_EXPR_STRING, out->string = string)

AstExpr *ast_expr_new_scope(Mempool *mempool, Slice slice, AstExpr *inner)
    CONSTRUCT(AST_EXPR_SCOPE, out->scope = inner)

AstExpr *ast_expr_new_callable(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr **args)
    CONSTRUCT(AST_EXPR_CALL,
        out->call.inner = inner;
        out->call.args = args;
    )

AstExpr *ast_expr_new_binop(Mempool *mempool, Slice slice, AstBinopKind kind, AstExpr *left, AstExpr *right)
    CONSTRUCT(AST_EXPR_BINOP,
        out->binop.kind = kind;
        out->binop.left = left;
        out->binop.right = right;
    )

AstExpr *ast_expr_new_struct(Mempool *mempool, Slice slice, AstType *type, AstExprStructField *fields_map)
    CONSTRUCT(AST_EXPR_STRUCT,
        out->structure.type = type;
        out->structure.fields_map = fields_map;
    )
