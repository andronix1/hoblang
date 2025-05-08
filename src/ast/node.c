#include "node.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/slice.h"
#include "core/vec.h"
#include "mempool.h"

#define FIELD(FROM, TO) out->FROM = TO;

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(AstNode, { \
    out->kind = KIND; \
    FIELDS; \
})

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
                vec_len(a->fun_decl.args) == vec_len(b->fun_decl.args);
            if (!meta_equals) {
                return false;
            }
            if (a->fun_decl.returns == NULL || a->fun_decl.returns == NULL) {
                if (a->fun_decl.returns != b->fun_decl.returns) {
                    return false;
                }
            } else if (!ast_type_eq(a->fun_decl.returns, b->fun_decl.returns)) {
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
        FIELD(type_decl.name, name)
        FIELD(type_decl.type, type)
        FIELD(type_decl.is_local, is_local))

AstNode *ast_node_new_fun_decl(Mempool *mempool, bool is_local, Slice name, AstFunArg *args, AstType *returns)
    CONSTRUCT(AST_NODE_FUN_DECL,
        FIELD(fun_decl.name, name)
        FIELD(fun_decl.returns, returns)
        FIELD(fun_decl.args, args)
        FIELD(fun_decl.is_local, is_local))
