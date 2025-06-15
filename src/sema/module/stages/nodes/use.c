#include "use.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"

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

bool sema_module_stage_init_use(SemaModule *module, AstUse *use) {
    sema_module_push_module_path(module, module, use->path, use->is_public);
    return true;
}
