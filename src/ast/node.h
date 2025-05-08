#pragma once

#include "ast/api/type.h"
#include "ast/body.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_NODE_TYPE_DECL,
    AST_NODE_FUN_DECL,
    AST_NODE_STMT,
} AstNodeKind;

typedef struct {
    bool is_local;
    Slice name;
    AstType *type;
} AstTypeDecl;

typedef struct {
    Slice name;
    AstType *type;
} AstFunArg;

typedef struct {
    bool is_local;
    Slice name;
    AstFunArg *args;
    AstType *returns;
    AstBody *body;
} AstFunDecl;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstTypeDecl type_decl;
        AstFunDecl fun_decl;
        AstStmt *stmt;
    };
} AstNode;

bool ast_node_eq(const AstNode *a, const AstNode *b);

AstFunArg ast_node_fun_arg(Slice name, AstType *type);

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type);
AstNode *ast_node_new_fun_decl(Mempool *mempool,
    bool is_local, Slice name,
    AstFunArg *args, AstType *returns,
    AstBody *body
);
AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt);
