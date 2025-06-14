#pragma once

#include "ast/api/expr.h"
#include "ast/api/type.h"
#include "ast/body.h"
#include "ast/api/generic.h"
#include "ast/global.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "ir/api/func.h"
#include "ir/api/local.h"
#include "sema/module/api/type.h"

typedef enum {
    AST_NODE_TYPE_DECL,
    AST_NODE_FUN_DECL,
    AST_NODE_VALUE_DECL,
    AST_NODE_EXTERNAL_DECL,
    AST_NODE_IMPORT,
    AST_NODE_STMT,
} AstNodeKind;

typedef struct {
    bool is_public;
    Slice name;
    AstType *type;
    AstGeneric *generics;

    struct {
        SemaTypeId type_id;
        SemaType *type;
    } sema;
} AstTypeDecl;

typedef struct {
    Slice name;
    AstType *type;
} AstFunArg;

typedef struct AstFunInfo {
    bool is_public;
    Slice name;
    AstFunArg *args;
    AstType *returns;

    struct {
        bool is;
        Slice of;
        bool by_ref;
        Slice self_name;

        struct {
            SemaType *ext_type;
        } sema;
    } ext;
} AstFunInfo;

typedef struct {
    AstGlobal *global;
    AstFunInfo *info;
    AstBody *body;

    struct {
        IrFuncId func_id;
        SemaType *type;
    } sema;
} AstFunDecl;

typedef enum {
    AST_VALUE_DECL_VAR,
    AST_VALUE_DECL_FINAL,
    AST_VALUE_DECL_CONST,
} AstValueDeclKind;

typedef struct AstValueInfo {
    bool is_public;
    Slice name;
    AstValueDeclKind kind;
    AstType *explicit_type;
} AstValueInfo;

typedef struct {
    AstGlobal *global;
    AstValueInfo *info;
    AstExpr *initializer;

    struct {
        SemaType *type;
        IrLocalId local_id;
    } sema;
} AstValueDecl;

typedef enum {
    AST_EXTERNAL_DECL_VALUE,
    AST_EXTERNAL_DECL_FUN,
} AstExternalDeclKind;

typedef struct {
    bool is_public;
    AstExternalDeclKind kind;

    bool has_alias;
    Slice alias;

    union {
        AstValueInfo *value;
        AstFunInfo *fun;
    };
} AstExternalDecl;

typedef enum {
    AST_IMPORT_MODULE,
    AST_IMPORT_LIBRARY,
} AstImportKind;

typedef struct {
    AstImportKind kind;
    bool is_public;
    union {
        struct {
            Slice path;
            Slice path_slice;
        } module;

        struct {
            Slice name;
        } library;
    };
    bool has_alias;
    Slice alias;
} AstImport;

typedef struct AstNode {
    AstNodeKind kind;
    Slice slice;

    union {
        AstImport import;
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
    bool is_public, Slice name,
    AstFunArg *args, AstType *returns
);
AstFunInfo *ast_ext_fun_info_new(Mempool *mempool,
    bool is_public, Slice name,
    AstFunArg *args, AstType *returns,
    Slice of, bool by_ref, Slice self_name
);
AstValueInfo *ast_value_info_new(Mempool *mempool,
    bool is_public, AstValueDeclKind kind, Slice name,
    AstType *explicit_type
);

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_public, Slice name, AstGeneric *generics, AstType *type);
AstNode *ast_node_new_fun_decl(Mempool *mempool, AstGlobal *global, AstFunInfo *info, AstBody *body);
AstNode *ast_node_new_value_decl(Mempool *mempool, AstGlobal *global, AstValueInfo *info, AstExpr *initializer);
AstNode *ast_node_new_external_value(Mempool *mempool, AstValueInfo *info, bool has_alias, Slice alias);
AstNode *ast_node_new_external_fun(Mempool *mempool, AstFunInfo *info, bool has_alias, Slice alias);
AstNode *ast_node_new_import_module(Mempool *mempool, bool is_public, Slice path_slice, Slice path);
AstNode *ast_node_new_import_module_alias(Mempool *mempool, bool is_public, Slice path_slice, Slice path, Slice alias);
AstNode *ast_node_new_import_library(Mempool *mempool, bool is_public, Slice library);
AstNode *ast_node_new_import_library_alias(Mempool *mempool, bool is_public, Slice library, Slice alias);
AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt);
