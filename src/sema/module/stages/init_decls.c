#include "init_decls.h"
#include "ast/node.h"
#include "core/null.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "core/assert.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static void sema_module_push_module_path(SemaModule *module, SemaModule *current_module, AstModulePath *path, bool is_public) {
    for (size_t i = 0; i < vec_len(path->module_path); i++) {
        SemaDecl *decl = RET_ON_NULL(sema_module_resolve_req_decl_from(current_module, module, path->module_path[i]));
        current_module = RET_ON_NULL(sema_value_is_module(decl->value));
    }
    if (path->is_combined) {
        for (size_t i = 0; i < vec_len(path->paths); i++) {
            sema_module_push_module_path(module, current_module, path->paths[i], is_public);
        }
    } else {
        SemaDecl *decl = RET_ON_NULL(sema_module_resolve_req_decl_from(current_module, module, path->single.decl_name));
        SemaDecl *aliased = sema_decl_new(module->mempool, is_public ? NULL : module, decl->value);
        sema_module_push_decl(module, path->single.has_alias ? path->single.alias : path->single.decl_name, aliased);
    }
}

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
        case AST_NODE_USE:
            sema_module_push_module_path(module, module, node->use.path, node->use.is_public);
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
