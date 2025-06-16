#pragma once

#include "ast/api/expr.h"
#include "ast/api/path.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "ast/api/type.h"
#include <stdbool.h>

typedef enum {
    AST_TYPE_STRUCT,
    AST_TYPE_PATH,
    AST_TYPE_ARRAY,
    AST_TYPE_POINTER,
} AstTypeKind;

typedef struct {
    bool is_public;
    AstType *type;
} AstStructField;

typedef struct {
    AstStructField *fields_map;
} AstStruct;

typedef struct {
    AstExpr *length;
    AstType *type;
} AstArray;

typedef struct AstType {
    AstTypeKind kind;
    Slice slice;

    union {
        AstStruct structure;
        AstArray array;
        AstPath *path;
        AstType *pointer_to;
        AstType *slice_of;
    };
} AstType;

bool ast_type_eq(const AstType *a, const AstType *b);

AstStructField ast_struct_field_new(bool is_public, AstType *type);
AstType *ast_type_new_struct(Mempool *mempool, AstStructField *fields_map);
AstType *ast_type_new_array(Mempool *mempool, AstExpr *length, AstType *type);
AstType *ast_type_new_path(Mempool *mempool, AstPath *path);
AstType *ast_type_new_pointer(Mempool *mempool, AstType *of);
AstType *ast_type_new_slice(Mempool *mempool, AstType *of);
