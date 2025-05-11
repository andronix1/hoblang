#pragma once

#include "ast/api/expr.h"
#include "ast/api/type.h"
#include "ast/body.h"
#include "ast/global.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"

typedef enum {
    AST_NODE_TYPE_DECL,
    AST_NODE_FUN_DECL,
    AST_NODE_VALUE_DECL,
    AST_NODE_EXTERNAL_DECL,
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

typedef struct AstFunInfo {
    Slice extension_of;
    bool is_local;
    Slice name;
    AstFunArg *args;
    AstType *returns;
} AstFunInfo;

typedef struct {
    AstGlobal *global;
    AstFunInfo *info;
    AstBody *body;
} AstFunDecl;

typedef enum {
    AST_VALUE_DECL_VAR,
    AST_VALUE_DECL_FINAL,
    AST_VALUE_DECL_CONST,
} AstValueDeclKind;

typedef struct AstValueInfo {
    bool is_local;
    Slice name;
    AstValueDeclKind kind;
    AstType *explicit_type;
} AstValueInfo;

typedef struct {
    AstGlobal *global;
    AstValueInfo *info;
    AstExpr *initializer;
} AstValueDecl;

typedef enum {
    AST_EXTERNAL_DECL_VALUE,
    AST_EXTERNAL_DECL_FUN,
} AstExternalDeclKind;

typedef struct {
    bool is_local;
    AstExternalDeclKind kind;

    bool has_alias;
    Slice alias;

    union {
        AstValueInfo *value;
        AstFunInfo *fun;
    };
} AstExternalDecl;

typedef struct AstNode {
    AstNodeKind kind;
    Slice slice;

    union {
        AstTypeDecl type_decl;
        AstFunDecl fun_decl;
        AstValueDecl value_decl;
        AstExternalDecl external_decl;
        AstStmt *stmt;
    };
} AstNode;

bool ast_node_eq(const AstNode *a, const AstNode *b);

AstFunArg ast_fun_arg_new(Slice name, AstType *type);
AstFunInfo *ast_fun_info_new(Mempool *mempool,
    bool is_local, Slice name,
    AstFunArg *args, AstType *returns
);
AstValueInfo *ast_value_info_new(Mempool *mempool,
    bool is_local, AstValueDeclKind kind, Slice name,
    AstType *explicit_type
);

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type);
AstNode *ast_node_new_fun_decl(Mempool *mempool, AstGlobal *global, AstFunInfo *info, AstBody *body);
AstNode *ast_node_new_value_decl(Mempool *mempool, AstGlobal *global, AstValueInfo *info, AstExpr *initializer);
AstNode *ast_node_new_external_value(Mempool *mempool, AstValueInfo *info, bool has_alias, Slice alias);
AstNode *ast_node_new_external_fun(Mempool *mempool, AstFunInfo *info, bool has_alias, Slice alias);
AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt);
