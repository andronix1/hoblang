#pragma once

#include "ast/api/expr.h"
#include "ast/api/type.h"
#include "ast/body.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_NODE_TYPE_DECL,
    AST_NODE_FUN_DECL,
    AST_NODE_VALUE_DECL,
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

typedef enum {
    AST_VALUE_DECL_VAR,
    AST_VALUE_DECL_FINAL,
    AST_VALUE_DECL_CONST,
} AstValueDeclKind;

typedef struct {
    bool is_local;
    Slice name;
    AstValueDeclKind kind;
    AstType *explicit_type;
    AstExpr *initializer;
} AstValueDecl;

typedef struct AstNode {
    AstNodeKind kind;

    union {
        AstTypeDecl type_decl;
        AstFunDecl fun_decl;
        AstValueDecl value_decl;
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
AstNode *ast_node_new_value_decl(Mempool *mempool,
    bool is_local,
    AstValueDeclKind kind, Slice name,
    AstType *explicit_type, AstExpr *initializer
);
AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt);
