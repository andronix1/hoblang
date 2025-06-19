#include "node.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "ast/generic.h"
#include "ast/global.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/slice.h"
#include "core/vec.h"
#include "core/mempool.h"
#include <stdio.h>
    
AstModulePath *ast_module_path_combined(Mempool *mempool, Slice *module_path, AstModulePath **paths)
    MEMPOOL_CONSTRUCT(AstModulePath,
        out->module_path = module_path;
        out->is_combined = true;
        out->paths = paths;
    )

AstModulePath *ast_module_path_single(Mempool *mempool, Slice *module_path, Slice decl_name, OptSlice alias)
    MEMPOOL_CONSTRUCT(AstModulePath,
        out->module_path = module_path;
        out->is_combined = false;
        out->single.alias = alias;
        out->single.decl_name = decl_name;
    )

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstNode, { \
    out->kind = KIND; \
    FIELDS; \
})

bool ast_global_eq(const AstGlobal *a, const AstGlobal *b) {
    return opt_slice_eq(&a->alias, &b->alias);
}

bool ast_generic_eq(const AstGeneric *a, const AstGeneric *b) {
    if (vec_len(a->params) != vec_len(b->params)) return false;
    for (size_t i = 0; i < vec_len(a->params); i++) {
        AstGenericParam *ap = &a->params[i];
        AstGenericParam *bp = &b->params[i];
        if (!slice_eq(ap->name, bp->name)) return false;
    }
    return true;
}

bool ast_value_info_eq(const AstValueInfo *a, const AstValueInfo *b) {
    return
        a->is_public == b->is_public &&
        a->kind == b->kind &&
        slice_eq(a->name, b->name) &&
        equals_nullable(a->explicit_type, b->explicit_type, (EqFunc)ast_type_eq);
}

bool ast_fun_info_eq(const AstFunInfo *a, const AstFunInfo *b) {
    bool meta_equals = a->is_public == b->is_public &&
        slice_eq(a->name, b->name) &&
        equals_nullable(a->returns, b->returns, (EqFunc)ast_type_eq) &&
        vec_len(a->args) == vec_len(b->args);
    if (!meta_equals) {
        return false;
    }
    for (size_t i = 0; i < vec_len(a->args); i++) {
        AstFunArg *aa = &a->args[i];
        AstFunArg *ba = &b->args[i];
        if (!ast_type_eq(aa->type, ba->type) || !slice_eq(aa->name, ba->name)) {
            return false;
        }
    }
    return true;
}

bool ast_module_paths_eq(const AstModulePath *a, const AstModulePath *b) {
    if (a->is_combined != b->is_combined || vec_len(a->module_path) != vec_len(b->module_path)) {
        return false;
    }
    for (size_t i = 0; i < vec_len(a->module_path); i++) {
        if (!slice_eq(a->module_path[i], b->module_path[i])) {
            return false;
        }
    }
    if (a->is_combined) {
        for (size_t i = 0; i < vec_len(a->paths); i++) {
            if (!ast_module_paths_eq(a->paths[i], b->paths[i])) {
                return false;
            }
        }
        return true;
    } else {
        return opt_slice_eq(&a->single.alias, &b->single.alias);
    }
}

bool ast_node_eq(const AstNode *a, const AstNode *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_NODE_TYPE_DECL:
            return a->type_decl.is_public == b->type_decl.is_public &&
                equals_nullable(a->type_decl.generics, b->type_decl.generics, (EqFunc)ast_generic_eq) &&
                slice_eq(a->type_decl.name, b->type_decl.name) &&
                ast_type_eq(a->type_decl.type, b->type_decl.type);
        case AST_NODE_FUN_DECL: {
            return
                ast_fun_info_eq(a->fun_decl.info, b->fun_decl.info) &&
                equals_nullable(a->fun_decl.global, b->fun_decl.global, (EqFunc)ast_global_eq) &&
                ast_body_eq(a->fun_decl.body, b->fun_decl.body);
        }
        case AST_NODE_VALUE_DECL:
            return
                ast_value_info_eq(a->value_decl.info, b->value_decl.info) &&
                equals_nullable(a->value_decl.global, b->value_decl.global, (EqFunc)ast_global_eq) &&
                equals_nullable(a->value_decl.initializer, b->value_decl.initializer, (EqFunc)ast_expr_eq);
        case AST_NODE_STMT: return ast_stmt_eq(a->stmt, b->stmt);
        case AST_NODE_EXTERNAL_DECL:
            if (a->external_decl.kind != b->external_decl.kind) {
                return false;
            }
            switch (a->external_decl.kind) {
                case AST_EXTERNAL_DECL_FUN: return ast_fun_info_eq(a->external_decl.fun, b->external_decl.fun);
                case AST_EXTERNAL_DECL_VALUE: return ast_value_info_eq(a->external_decl.value, b->external_decl.value);
            }
            UNREACHABLE;
        case AST_NODE_USE:
            return a->use.is_public != b->use.is_public &&
                ast_module_paths_eq(a->use.path, b->use.path);
        case AST_NODE_IMPORT:
            if (a->import.is_public != b->import.is_public ||
                a->import.kind != b->import.kind ||
                a->import.has_alias != b->import.has_alias ||
                (a->import.has_alias && !slice_eq(a->import.alias, b->import.alias))) {
                return false;
            }
            switch (a->import.kind) {
                case AST_IMPORT_MODULE: return slice_eq(a->import.module.path, b->import.module.path);
                case AST_IMPORT_LIBRARY: return slice_eq(a->import.library.name, b->import.library.name);
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}

AstFunArg ast_fun_arg_new(Slice name, AstType *type) {
    AstFunArg arg = {
        .name = name,
        .type = type,
    };
    return arg;
}

AstFunInfo *ast_fun_info_new(Mempool *mempool,
    bool is_public, Slice name,
    AstFunArg *args, AstType *returns
) MEMPOOL_CONSTRUCT(AstFunInfo,
    out->is_public = is_public;
    out->ext.is = false;
    out->name = name;
    out->args = args;
    out->returns = returns;
)

AstFunInfo *ast_ext_fun_info_new(Mempool *mempool,
    bool is_public, Slice name,
    AstFunArg *args, AstType *returns,
    Slice of, bool by_ref, Slice self_name
) MEMPOOL_CONSTRUCT(AstFunInfo,
    out->is_public = is_public;
    out->ext.is = true;
    out->ext.of = of;
    out->ext.self_name = self_name;
    out->ext.by_ref = by_ref;
    out->name = name;
    out->args = args;
    out->returns = returns;
)

AstValueInfo *ast_value_info_new(Mempool *mempool,
    bool is_public, AstValueDeclKind kind, Slice name,
    AstType *explicit_type
) MEMPOOL_CONSTRUCT(AstValueInfo,
    out->is_public = is_public;
    out->kind = kind;
    out->name = name;
    out->explicit_type = explicit_type;
)

AstNode *ast_node_new_use(Mempool *mempool, bool is_public, AstModulePath *path)
    CONSTRUCT(AST_NODE_USE,
        out->use.is_public = is_public;
        out->use.path = path;
    )

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_public, Slice name, AstGeneric *generics, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        out->type_decl.name = name;
        out->type_decl.type = type;
        out->type_decl.generics = generics;
        out->type_decl.is_public = is_public;
    )

AstNode *ast_node_new_fun_decl(Mempool *mempool,
    AstGlobal *global,
    AstFunInfo *info,
    AstBody *body
)
    CONSTRUCT(AST_NODE_FUN_DECL,
        out->fun_decl.global = global;
        out->fun_decl.info = info;
        out->fun_decl.body = body;
    )

AstNode *ast_node_new_import_module(Mempool *mempool, bool is_public, Slice path_slice, Slice path)
    CONSTRUCT(AST_NODE_IMPORT,
        out->import.kind = AST_IMPORT_MODULE;
        out->import.is_public = is_public;
        out->import.module.path_slice = path_slice;
        out->import.module.path = path;
        out->import.has_alias = false;
    )

AstNode *ast_node_new_import_module_alias(Mempool *mempool, bool is_public, Slice path_slice, Slice path, Slice alias)
    CONSTRUCT(AST_NODE_IMPORT,
        out->import.kind = AST_IMPORT_MODULE;
        out->import.is_public = is_public;
        out->import.module.path_slice = path_slice;
        out->import.module.path = path;
        out->import.has_alias = true;
        out->import.alias = alias;
    )

AstNode *ast_node_new_import_library(Mempool *mempool, bool is_public, Slice library)
    CONSTRUCT(AST_NODE_IMPORT,
        out->import.kind = AST_IMPORT_LIBRARY;
        out->import.is_public = is_public;
        out->import.library.name = library;
        out->import.has_alias = false;
    )

AstNode *ast_node_new_import_library_alias(Mempool *mempool, bool is_public, Slice library, Slice alias)
    CONSTRUCT(AST_NODE_IMPORT,
        out->import.kind = AST_IMPORT_LIBRARY;
        out->import.is_public = is_public;
        out->import.library.name = library;
        out->import.has_alias = true;
        out->import.alias = alias;
    )

AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt)
    CONSTRUCT(AST_NODE_STMT, out->stmt = stmt;)

AstNode *ast_node_new_value_decl(Mempool *mempool,
    AstGlobal *global,
    AstValueInfo *info,
    AstExpr *initializer
)
    CONSTRUCT(AST_NODE_VALUE_DECL,
        out->value_decl.global = global;
        out->value_decl.info = info;
        out->value_decl.initializer = initializer;
    )

AstNode *ast_node_new_external_value(Mempool *mempool, AstValueInfo *info, OptSlice alias)
    CONSTRUCT(AST_NODE_EXTERNAL_DECL,
        out->external_decl.kind = AST_EXTERNAL_DECL_VALUE;
        out->external_decl.value = info;
        out->external_decl.alias = alias;
    )

AstNode *ast_node_new_external_fun(Mempool *mempool, AstFunInfo *info, OptSlice alias)
    CONSTRUCT(AST_NODE_EXTERNAL_DECL,
        out->external_decl.kind = AST_EXTERNAL_DECL_FUN;
        out->external_decl.fun = info;
        out->external_decl.alias = alias;
    )
