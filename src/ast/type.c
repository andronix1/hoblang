#include "type.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/slice.h"
#include "core/vec.h"
#include "core/mempool.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstType, out->kind = KIND; FIELDS)

AstStructField ast_struct_field_new(bool is_local, AstType *type) {
    AstStructField field = {
        .is_local = is_local,
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
                if (af->value.is_local != bf->value.is_local) {
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
        case AST_TYPE_SLICE: return ast_type_eq(a->slice_of, b->slice_of);
    }
    UNREACHABLE;
}

AstType *ast_type_new_struct(Mempool *mempool, AstStructField *fields_map)
    CONSTRUCT(AST_TYPE_STRUCT, out->structure.fields_map = fields_map;)

AstType *ast_type_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_TYPE_PATH, out->path = path;)

AstType *ast_type_new_pointer(Mempool *mempool, AstType *to)
    CONSTRUCT(AST_TYPE_POINTER, out->pointer_to = to;)

AstType *ast_type_new_slice(Mempool *mempool, AstType *of)
    CONSTRUCT(AST_TYPE_SLICE, out->slice_of = of;)
