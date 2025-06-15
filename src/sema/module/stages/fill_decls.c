#include "fill_decls.h"
#include "ast/node.h"
#include "core/assert.h"
#include "sema/module/module.h"
#include "sema/module/stages/nodes/external.h"
#include "sema/module/stages/nodes/fun.h"
#include "sema/module/stages/nodes/type.h"
#include "sema/module/stages/nodes/value.h"

bool sema_module_fill_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: return sema_module_stage_fill_type_decl(module, &node->type_decl);
        case AST_NODE_EXTERNAL_DECL: return sema_module_stage_fill_external(module, &node->external_decl);
        case AST_NODE_FUN_DECL: return sema_module_stage_fill_fun(module, &node->fun_decl);
        case AST_NODE_VALUE_DECL: return sema_module_stage_fill_value(module, &node->value_decl);
        case AST_NODE_IMPORT: case AST_NODE_USE: case AST_NODE_STMT: return true;
    }
    UNREACHABLE;
}
