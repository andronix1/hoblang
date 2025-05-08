#pragma once

#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_NODE_TYPE_DECL,
} AstNodeKind;

typedef struct {
    bool is_local;
    Slice name;
    AstType *type;
} AstTypeDecl;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstTypeDecl type_decl;
    };
} AstNode;

bool ast_node_eq(const AstNode *a, const AstNode *b);

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type);
