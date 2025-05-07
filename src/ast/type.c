#include "type.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/slice.h"
#include "core/vec.h"
#include "mempool.h"

#define FIELD(FROM, TO) out->FROM = TO;

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstType, { \
    out->kind = KIND; \
    FIELDS; \
})

AstStructField ast_struct_field_new(Slice name, AstType *type) {
    AstStructField field = {
        .name = name,
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
            if (vec_len(a->structure.fields) != vec_len(b->structure.fields)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields); i++) {
                AstStructField *af = &a->structure.fields[i];
                AstStructField *bf = &b->structure.fields[i];
                if (!slice_eq(af->name, bf->name)) {
                    return false;
                }
                if (!ast_type_eq(af->type, bf->type)) {
                    return false;
                }
            }
            return true;
        }
    }
    UNREACHABLE;
}

AstType *ast_type_new_struct(Mempool *mempool, AstStructField *fields)
    CONSTRUCT(AST_TYPE_STRUCT, FIELD(structure.fields, fields))

AstType *ast_type_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_TYPE_PATH, FIELD(path, path))
