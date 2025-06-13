#include "emit_bodies.h"
#include "ast/api/body.h"
#include "ast/node.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/stmt/code.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stages/stages.h"
#include "sema/module/stmts/stmt.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static inline IrCode *sema_module_emit_code(SemaModule *module, AstBody *body) {
    sema_module_push_scope(module);
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        for (size_t j = 0; j < sema_stages_count; j++) {
            if (!sema_stages[j](module, body->nodes[i])) {
                break;
            }
        }
    }
    IrStmt **stmts = sema_ss_get_stmts(module->ss);
    sema_module_pop_scope(module);
    return ir_code_new(module->mempool, stmts);
}

void sema_module_emit_func_body(SemaModule *module, AstFunDecl *func) {
    SemaType *type = func->sema.type;
    assert(type->kind == SEMA_TYPE_FUNCTION);
    SemaScopeStack *old_ss = sema_module_swap_ss(module,
        sema_scope_stack_new(module->mempool, func->sema.func_id, type->function.returns));

    sema_module_push_scope(module);
    if (func->info->ext.is) {
        sema_module_push_decl(module, func->info->ext.self_name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
                module->mempool,
                SEMA_RUNTIME_FINAL,
                type->function.args[0],
                ir_func_arg_local_id(module->ir, func->sema.func_id, 0)
            )
        ));
    }
    for (size_t i = 0; i < vec_len(func->info->args); i++) {
        sema_module_push_decl(module, func->info->args[i].name,
            sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
                module->mempool,
                SEMA_RUNTIME_VAR,
                type->function.args[i + func->info->ext.is],
                ir_func_arg_local_id(module->ir, func->sema.func_id, i + func->info->ext.is)
            )
        ));
    }
    ir_init_func_body(module->ir, func->sema.func_id,
        sema_module_emit_code(module, func->body));
    sema_module_pop_scope(module);

    sema_module_swap_ss(module, old_ss);
}

bool sema_module_emit_node_body(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_FUN_DECL:
            sema_module_emit_func_body(module, &node->fun_decl);
            return true;

        case AST_NODE_STMT:
            sema_module_emit_stmt(module, node->stmt);
            return true;
        case AST_NODE_VALUE_DECL:
        case AST_NODE_TYPE_DECL:
        case AST_NODE_EXTERNAL_DECL:
        case AST_NODE_IMPORT:
            return true;
    }
    if (node->kind != AST_NODE_FUN_DECL) {
        return true;
    }
    return true;
}

