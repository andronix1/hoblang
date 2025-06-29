#include "expr.h"
#include "ast/type.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/null.h"
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
        case AST_EXPR_INNER_PATH:
            return ast_path_eq(a->inner_path.path, b->inner_path.path) &&
                    ast_expr_eq(a->inner_path.inner, b->inner_path.inner);
        case AST_EXPR_CALL:
            if (!ast_expr_eq(a->call.inner, b->call.inner) ||
                vec_len(a->call.args) != vec_len(b->call.args)) return false;
            for (size_t i = 0; i < vec_len(a->call.args); i++) {
                if (!ast_expr_eq(a->call.args[i], b->call.args[i])) {
                    return false;
                }
            }
            return true;
        case AST_EXPR_FUNCTION:
            if (!equals_nullable(a->func.returns, b->func.returns, (EqFunc)ast_type_eq) ||
                vec_len(a->func.args) != vec_len(b->func.args)) return false;
            for (size_t i = 0; i < vec_len(a->func.args); i++) {
                if (!ast_type_eq(a->func.args[i].type, b->func.args[i].type) ||
                    !slice_eq(a->func.args[i].name, b->func.args[i].name)) {
                    return false;
                }
            }
            return true;
        case AST_EXPR_CHAR: return a->character == b->character;
        case AST_EXPR_INTEGER: return a->integer == b->integer;
        case AST_EXPR_BOOL: return a->boolean == b->boolean;
        case AST_EXPR_SCOPE: return ast_expr_eq(a->scope, b->scope);
        case AST_EXPR_BINOP: return
            a->binop.kind.kind == b->binop.kind.kind &&
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
        case AST_EXPR_ARRAY:
            if (!ast_type_eq(a->array.type, b->array.type)) return false;
            if (vec_len(a->array.elements) != vec_len(b->array.elements)) return false;
            for (size_t i = 0; i < vec_len(a->array.elements); i++) {
                if (!ast_expr_eq(a->array.elements[i], b->array.elements[i])) return false;
            }
            return true;
        case AST_EXPR_AS: return ast_expr_eq(a->as.inner, b->as.inner) && ast_type_eq(a->as.type, b->as.type);
        case AST_EXPR_IDX: return ast_expr_eq(a->idx.inner, b->idx.inner) && ast_expr_eq(a->idx.idx, b->idx.idx);
        case AST_EXPR_NOT: return ast_expr_eq(a->not_inner, b->not_inner);
        case AST_EXPR_NEG: return ast_expr_eq(a->neg_inner, b->neg_inner);
        case AST_EXPR_TAKE_REF: return ast_expr_eq(a->take_ref_inner, b->take_ref_inner);
        case AST_EXPR_FLOAT: return a->float_value == b->float_value;
        case AST_EXPR_UNDEFINED: return true;
    }
    UNREACHABLE;
}

AstExpr *ast_expr_new_undefined(Mempool *mempool, Slice slice)
    CONSTRUCT(AST_EXPR_UNDEFINED,)

AstExpr *ast_expr_new_array(Mempool *mempool, Slice slice, AstType *type, AstExpr **elements)
    CONSTRUCT(AST_EXPR_ARRAY,
        out->array.elements = elements;
        out->array.type = type;
    )

AstExpr *ast_expr_new_bool(Mempool *mempool, Slice slice, bool boolean)
    CONSTRUCT(AST_EXPR_BOOL, out->boolean = boolean)

AstExpr *ast_expr_new_char(Mempool *mempool, Slice slice, char c)
    CONSTRUCT(AST_EXPR_CHAR, out->character = c)

AstExpr *ast_expr_new_path(Mempool *mempool, Slice slice, AstPath *path)
    CONSTRUCT(AST_EXPR_PATH, out->path = path)

AstExpr *ast_expr_new_inner_path(Mempool *mempool, Slice slice, AstExpr *inner, AstPath *path)
    CONSTRUCT(AST_EXPR_INNER_PATH,
        out->inner_path.inner = inner;
        out->inner_path.path = path;
    )

AstExpr *ast_expr_new_idx(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr *idx)
    CONSTRUCT(AST_EXPR_IDX,
        out->idx.inner = inner;
        out->idx.idx = idx;
    )

AstExpr *ast_expr_new_neg(Mempool *mempool, Slice slice, AstExpr *inner)
    CONSTRUCT(AST_EXPR_NEG, out->neg_inner = inner)

AstExpr *ast_expr_new_not(Mempool *mempool, Slice slice, AstExpr *inner)
    CONSTRUCT(AST_EXPR_NOT, out->not_inner = inner)

AstExpr *ast_expr_new_take_ref(Mempool *mempool, Slice slice, AstExpr *inner)
    CONSTRUCT(AST_EXPR_TAKE_REF, out->take_ref_inner = inner)

AstExpr *ast_expr_new_as(Mempool *mempool, Slice slice, Slice as_slice, AstExpr *inner, AstType *as)
    CONSTRUCT(AST_EXPR_AS,
        out->as.inner = inner;
        out->as.type = as;
        out->as.slice = as_slice;
    )

AstExpr *ast_expr_new_integer(Mempool *mempool, Slice slice, uint64_t integer)
    CONSTRUCT(AST_EXPR_INTEGER, out->integer = integer)

AstExpr *ast_expr_new_float(Mempool *mempool, Slice slice, long double value)
    CONSTRUCT(AST_EXPR_FLOAT, out->float_value = value)

AstExpr *ast_expr_new_string(Mempool *mempool, Slice slice, Slice string)
    CONSTRUCT(AST_EXPR_STRING, out->string = string)

AstExpr *ast_expr_new_scope(Mempool *mempool, Slice slice, AstExpr *inner)
    CONSTRUCT(AST_EXPR_SCOPE, out->scope = inner)

AstExpr *ast_expr_new_callable(Mempool *mempool, Slice slice, AstExpr *inner, AstExpr **args)
    CONSTRUCT(AST_EXPR_CALL,
        out->call.inner = inner;
        out->call.args = args;
    )

AstExpr *ast_expr_new_function(Mempool *mempool, Slice slice, AstExprFuncArg *args, AstType *returns, AstBody *body)
    CONSTRUCT(AST_EXPR_FUNCTION,
        out->func.args = args;
        out->func.returns = returns;
        out->func.body = body;
    )

AstExpr *ast_expr_new_binop(Mempool *mempool, Slice slice, AstBinopKind binop, AstExpr *left, AstExpr *right)
    CONSTRUCT(AST_EXPR_BINOP,
        out->binop.kind = binop;
        out->binop.left = left;
        out->binop.right = right;
    )

AstExpr *ast_expr_new_struct(Mempool *mempool, Slice slice, AstType *type, AstExprStructField *fields_map)
    CONSTRUCT(AST_EXPR_STRUCT,
        out->structure.type = type;
        out->structure.fields_map = fields_map;
    )
