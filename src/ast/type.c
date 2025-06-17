#include "type.h"
#include "ast/expr.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "core/mempool.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstType, out->kind = KIND; FIELDS)

AstStructField ast_struct_field_new(bool is_public, AstType *type) {
    AstStructField field = {
        .is_public = is_public,
        .type = type,
    };
    return field;
}

bool ast_type_eq(const AstType *a, const AstType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_TYPE_PATH: return ast_path_eq(a->path, b->path);
        case AST_TYPE_STRUCT: {
            if (vec_len(a->structure.fields_map) != vec_len(b->structure.fields_map)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields_map); i++) {
                keymap_at(a->structure.fields_map, i, af);
                keymap_at(a->structure.fields_map, i, bf);
                if (af->value.is_public != bf->value.is_public) {
                    return false;
                }
                if (!slice_eq(af->key, bf->key)) {
                    return false;
                }
                if (!ast_type_eq(af->value.type, bf->value.type)) {
                    return false;
                }
            }
            return true;
        }
        case AST_TYPE_POINTER: return ast_type_eq(a->pointer_to, b->pointer_to);
        case AST_TYPE_ARRAY:
            return ast_expr_eq(a->array.length, a->array.length) && ast_type_eq(a->array.type, b->array.type);
        case AST_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->function.args); i++) {
                if (!ast_type_eq(a->function.args[i], b->function.args[i])) {
                    return false;
                }
            }
            return equals_nullable(a->function.returns, b->function.returns, (EqFunc)ast_type_eq);
    }
    UNREACHABLE;
}

AstType *ast_type_new_function(Mempool *mempool, AstType **args, AstType *returns)
    CONSTRUCT(AST_TYPE_FUNCTION,
        out->function.args = args;
        out->function.returns = returns;
    )

AstType *ast_type_new_array(Mempool *mempool, AstExpr *length, AstType *type)
    CONSTRUCT(AST_TYPE_ARRAY,
        out->array.length = length;
        out->array.type = type;
    )

AstType *ast_type_new_struct(Mempool *mempool, AstStructField *fields_map)
    CONSTRUCT(AST_TYPE_STRUCT, out->structure.fields_map = fields_map;)

AstType *ast_type_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_TYPE_PATH, out->path = path; out->slice = ast_path_slice(path))

AstType *ast_type_new_pointer(Mempool *mempool, AstType *to)
    CONSTRUCT(AST_TYPE_POINTER, out->pointer_to = to;)
