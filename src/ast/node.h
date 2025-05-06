#pragma once

#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_NODE_TYPE_DECL,
} AstNodeKind;

typedef struct {
    Slice name;
    AstType *type;
} AstTypeDecl;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstTypeDecl decl;
    };
} AstNode;

AstNode *ast_node_new_type_decl(Mempool *mempool, Slice name, AstType *type);
