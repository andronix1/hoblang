#include "node.h"
#include "ast/body.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/type.h"
#include "core/assert.h"
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
            bool meta_equals = a->fun_decl.is_local == b->fun_decl.is_local &&
                slice_eq(a->fun_decl.name, b->fun_decl.name) &&
                ast_body_eq(a->fun_decl.body, b->fun_decl.body) &&
                equals_nullable(a->fun_decl.returns, b->fun_decl.returns, (EqFunc)ast_type_eq) &&
                vec_len(a->fun_decl.args) == vec_len(b->fun_decl.args);
            if (!meta_equals) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->fun_decl.args); i++) {
                AstFunArg *aa = &a->fun_decl.args[i];
                AstFunArg *ba = &b->fun_decl.args[i];
                if (!ast_type_eq(aa->type, ba->type) || !slice_eq(aa->name, ba->name)) {
                    return false;
                }
            }
            return true;
        }
        case AST_NODE_VALUE_DECL:
            return
                a->value_decl.is_local == b->value_decl.is_local &&
                a->value_decl.kind == b->value_decl.kind &&
                slice_eq(a->value_decl.name, b->value_decl.name) &&
                equals_nullable(a->value_decl.explicit_type, b->value_decl.explicit_type, (EqFunc)ast_type_eq) &&
                equals_nullable(a->value_decl.initializer, b->value_decl.initializer, (EqFunc)ast_expr_eq);
        case AST_NODE_STMT: return ast_stmt_eq(a->stmt, b->stmt);
    }
    UNREACHABLE;
}

AstFunArg ast_node_fun_arg(Slice name, AstType *type) {
    AstFunArg arg = {
        .name = name,
        .type = type,
    };
    return arg;
}

AstNode *ast_node_new_type_decl(Mempool *mempool, bool is_local, Slice name, AstType *type)
    CONSTRUCT(AST_NODE_TYPE_DECL,
        out->type_decl.name = name;
        out->type_decl.type = type;
        out->type_decl.is_local = is_local;
    )

AstNode *ast_node_new_fun_decl(Mempool *mempool,
    bool is_local, Slice name,
    AstFunArg *args, AstType *returns,
    AstBody *body
)
    CONSTRUCT(AST_NODE_FUN_DECL,
        out->fun_decl.name = name;
        out->fun_decl.returns = returns;
        out->fun_decl.args = args;
        out->fun_decl.is_local = is_local;
        out->fun_decl.body = body;
    )

AstNode *ast_node_new_stmt(Mempool *mempool, AstStmt *stmt)
    CONSTRUCT(AST_NODE_STMT, out->stmt = stmt;)

AstNode *ast_node_new_value_decl(Mempool *mempool,
    bool is_local,
    AstValueDeclKind kind, Slice name,
    AstType *explicit_type, AstExpr *initializer
)
    CONSTRUCT(AST_NODE_VALUE_DECL,
        out->value_decl.is_local = is_local;
        out->value_decl.name = name;
        out->value_decl.kind = kind;
        out->value_decl.explicit_type = explicit_type;
        out->value_decl.initializer = initializer;
    )
