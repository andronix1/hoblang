#include "emit_bodies.h"
#include "ast/node.h"
#include "sema/module/module.h"
#include "sema/module/stages/nodes/fun.h"
#include "sema/module/stages/nodes/value.h"
#include "sema/module/stmts/stmt.h"

bool sema_module_emit_node_body(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_FUN_DECL: return sema_module_stage_emit_fun(module, &node->fun_decl);
        case AST_NODE_STMT: sema_module_emit_stmt(module, node->stmt); return true;
        case AST_NODE_VALUE_DECL: return sema_module_stage_emit_value(module, &node->value_decl);
        case AST_NODE_TYPE_DECL:
        case AST_NODE_EXTERNAL_DECL:
        case AST_NODE_IMPORT:
        case AST_NODE_USE:
            return true;
    }
    if (node->kind != AST_NODE_FUN_DECL) {
        return true;
    }
    return true;
}

