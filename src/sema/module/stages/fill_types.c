#include "fill_types.h"
#include "ast/node.h"
#include "sema/module/stages/nodes/type.h"

bool sema_module_fill_types(SemaModule *module, AstNode *node) {
    if (node->kind == AST_NODE_TYPE_DECL) {
        return sema_module_stage_fill_type_decl(module, &node->type_decl);
    }
    return true;
}
