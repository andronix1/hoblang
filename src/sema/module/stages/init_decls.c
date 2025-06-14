#include "init_decls.h"
#include "ast/node.h"
#include "core/null.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "core/assert.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

bool sema_module_init_node_decls(SemaModule *module, AstNode *node) {
    switch (node->kind) {
        case AST_NODE_TYPE_DECL: {
            SemaTypeId id = node->type_decl.sema.type_id = sema_module_register_type_alias(module);
            IrTypeId ir_id = module->types[id].id;
            sema_module_push_decl(module, node->type_decl.name, sema_decl_new(module->mempool,
                node->type_decl.is_public ? NULL : module,
                sema_value_new_type(module->mempool, node->type_decl.sema.type = sema_type_new_alias(module->mempool,
                    sema_type_new_record(module, id), sema_type_alias_new(module->mempool, ir_id)))));
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
            switch (node->import.kind) {
                case AST_IMPORT_MODULE: {
                    SemaModule *imported = NOT_NULL(sema_project_add_module(module->project, sema_module_file_path(module),
                        mempool_slice_to_cstr(module->mempool, node->import.module.path)));
                    if (!node->import.has_alias) {
                        sema_module_err(module, node->slice, "import alias must be specified for importing module");
                        return false;
                    }
                    sema_module_push_decl(module, node->import.alias, sema_decl_new(module->mempool,
                        node->import.is_public ? NULL : module,
                        sema_value_new_module(module->mempool, imported)));
                    return true;
                }
                case AST_IMPORT_LIBRARY: {
                    SemaModule *imported = NOT_NULL(sema_project_add_library(module->project, node->import.library.name));
                    sema_module_push_decl(module,
                        node->import.has_alias ? node->import.alias : node->import.library.name,
                        sema_decl_new(module->mempool, node->import.is_public ? NULL : module,
                            sema_value_new_module(module->mempool, imported)));
                    return true;
                }
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}
