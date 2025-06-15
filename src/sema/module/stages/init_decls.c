#include "init_decls.h"
#include "ast/node.h"
#include "sema/module/module.h"
#include "core/assert.h"
#include "sema/module/stages/nodes/import.h"
#include "sema/module/stages/nodes/stmt.h"
#include "sema/module/stages/nodes/type.h"
#include "sema/module/stages/nodes/use.h"

bool sema_module_init_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: return sema_module_stage_init_type_decl(module, &node->type_decl);
        case AST_NODE_STMT: return sema_module_stage_init_stmt(module, node->slice);
        case AST_NODE_USE: return sema_module_stage_init_use(module, &node->use);
        case AST_NODE_IMPORT: return sema_module_stage_init_import(module, &node->import);
        case AST_NODE_EXTERNAL_DECL: case AST_NODE_FUN_DECL: case AST_NODE_VALUE_DECL: return true;
    }
    UNREACHABLE;
}
