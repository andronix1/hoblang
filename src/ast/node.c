#include "node.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "ast/global.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/mempool.h"
#include "core/slice.h"
#include "core/vec.h"
#include "mempool.h"

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstNode, { \
    out->kind = KIND; \
    FIELDS; \
})

typedef bool (*EqFunc)(const void *a, const void *b);
static bool equals_nullable(const void *a, const void *b, EqFunc eq) {
    if (a == NULL || a == NULL) {
        if (a != b) {
            return false;
        }
    } else if (!eq(a, b)) {
        return false;
    }
    return true;
}

bool ast_global_eq(const AstGlobal *a, const AstGlobal *b) {
    return
        a->has_alias == b->has_alias &&
        (!a->has_alias || slice_eq(a->alias, b->alias));
}

bool ast_value_info_eq(const AstValueInfo *a, const AstValueInfo *b) {
    return
        a->is_local == b->is_local &&
        a->kind == b->kind &&
        slice_eq(a->name, b->name) &&
        equals_nullable(a->explicit_type, b->explicit_type, (EqFunc)ast_type_eq);
}

bool ast_fun_info_eq(const AstFunInfo *a, const AstFunInfo *b) {
    bool meta_equals = a->is_local == b->is_local &&
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

bool ast_node_eq(const AstNode *a, const AstNode *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case AST_NODE_TYPE_DECL:
            return a->type_decl.is_local == b->type_decl.is_local &&
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
    bool is_local, Slice name,
    AstFunArg *args, AstType *returns
) MEMPOOL_CONSTRUCT(AstFunInfo,
    out->is_local = is_local;
    out->name = name;
    out->args = args;
    out->returns = returns;
)

AstValueInfo *ast_value_info_new(Mempool *mempool,
    bool is_local, AstValueDeclKind kind, Slice name,
    AstType *explicit_type
) MEMPOOL_CONSTRUCT(AstValueInfo,
    out->is_local = is_local;
    out->kind = kind;
    out->name = name;
    out->explicit_type = explicit_type;
)

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        out->type_decl.name = name;
        out->type_decl.type = type;
        out->type_decl.is_local = is_local;
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

AstNode *ast_node_new_external_value(Mempool *mempool, AstValueInfo *info, bool has_alias, Slice alias)
    CONSTRUCT(AST_NODE_EXTERNAL_DECL,
        out->external_decl.kind = AST_EXTERNAL_DECL_VALUE;
        out->external_decl.value = info;
        out->external_decl.has_alias = has_alias;
        out->external_decl.alias = alias;
    )

AstNode *ast_node_new_external_fun(Mempool *mempool, AstFunInfo *info, bool has_alias, Slice alias)
    CONSTRUCT(AST_NODE_EXTERNAL_DECL,
        out->external_decl.kind = AST_EXTERNAL_DECL_FUN;
        out->external_decl.fun = info;
        out->external_decl.has_alias = has_alias;
        out->external_decl.alias = alias;
    )
