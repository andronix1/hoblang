#include "init_decls.h"
#include "ast/node.h"
#include "sema/module/module.h"
#include "core/assert.h"

bool sema_module_init_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            AstTypeDecl *info = &node->type_decl;
            info->sema.type_id = sema_module_register_type_alias(module);
            return true;
        }
        case AST_NODE_EXTERNAL_DECL:
        case AST_NODE_FUN_DECL:
        case AST_NODE_VALUE_DECL:
            return true;
        case AST_NODE_STMT:
            if (sema_module_is_global_scope(module)) {
                sema_module_err(module, node->slice, "statements are not allowed in global scope");
                return false;
            }
            return true;
        case AST_NODE_IMPORT:
            TODO;
    }
    UNREACHABLE;
}
