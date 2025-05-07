#pragma once

#include "ast/api/path.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "ast/api/type.h"

typedef enum {
    AST_TYPE_STRUCT,
    AST_TYPE_PATH,
} AstTypeKind;

typedef struct {
    Slice name;
    AstType *type;
} AstStructField;

typedef struct {
    AstStructField *fields;
} AstStruct;

typedef struct AstType {
    AstTypeKind kind;

    union {
        AstStruct structure;
        AstPath *path;
    };
} AstType;

bool ast_type_eq(const AstType *a, const AstType *b);

AstStructField ast_struct_field_new(Slice name, AstType *type);
AstType *ast_type_new_struct(Mempool *pool, AstStructField *fields);
AstType *ast_type_new_path(Mempool *pool, AstPath *path);
