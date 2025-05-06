#include "type.h"
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

AstType *ast_type_new_struct(Mempool *mempool, AstStructField *fields)
    CONSTRUCT(AST_TYPE_STRUCT, FIELD(structure.fields, fields))

AstType *ast_type_new_path(Mempool *mempool, AstPath *path)
    CONSTRUCT(AST_TYPE_PATH, FIELD(path, path))
