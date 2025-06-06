#include "emit_bodies.h"
#include "ast/api/body.h"
#include "ast/node.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/stmt/code.h"
#include "sema/module/module.h"
#include "sema/module/stages/stages.h"

static inline IrCode *sema_module_emit_code(SemaModule *module, AstBody *body) {
    IrStmt **stmts = vec_new_in(module->mempool, IrStmt*);
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        for (size_t j = 0; j < sema_stages_count; j++) {
            if (!sema_stages[j](module, body->nodes[i])) {
                break;
            }
        }
    }
    return ir_code_new(module->mempool, stmts);
}

bool sema_module_emit_node_body(SemaModule *module, AstNode *node) {
    if (node->kind != AST_NODE_FUN_DECL) {
        return true;
    }
    AstFunDecl *func = &node->fun_decl;
    ir_init_func_body(module->ir, func->sema.func_id,
        NOT_NULL(sema_module_emit_code(module, func->body)));
    return true;
}
