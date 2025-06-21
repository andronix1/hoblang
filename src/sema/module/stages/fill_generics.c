#include "ast/node.h"
#include "sema/module/module.h"
#include "core/assert.h"
#include "sema/module/stages/nodes/type.h"

bool sema_module_fill_node_generics(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL:
            return sema_module_stage_fill_type_generics(module, &node->type_decl);
        case AST_NODE_STMT:
        case AST_NODE_USE:
        case AST_NODE_IMPORT:
        case AST_NODE_EXTERNAL_DECL:
        case AST_NODE_FUN_DECL:
        case AST_NODE_VALUE_DECL:
            return true;
    }
    UNREACHABLE;
}
