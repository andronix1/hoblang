#include "fun_info.h"
#include "ast/path.h"
#include "core/null.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "ast/node.h"
#include "sema/module/type.h"

SemaType *sema_func_info_type(SemaModule *module, AstFunInfo *info) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_reserve(args, vec_len(info->args) + info->ext.is);

    info->ext.sema.ext_type = NULL;
    if (info->ext.is) {
        AstType *of = ast_type_new_path(module->mempool, ast_path_new(module->mempool, vec_create_in(module->mempool,
            ast_path_segment_new_ident(info->ext.of))));
        SemaType *ext_of = info->ext.sema.ext_type = NOT_NULL(sema_module_type(module, of));
        vec_push(args, info->ext.by_ref ? sema_type_new_pointer(module, ext_of) : ext_of);
    }

    for (size_t i = 0; i < vec_len(info->args); i++) {
        vec_push(args, NOT_NULL(sema_module_type(module, info->args[i].type)));
    }

    SemaType *returns = NOT_NULL(sema_module_opt_type(module, info->returns));
    return sema_type_new_function(module, args, returns);
}

void sema_module_push_fun_info_decl(SemaModule *module, AstFunInfo *info, SemaDecl *decl) {
    if (info->ext.is) {
        SemaType *ext_type = info->ext.sema.ext_type;
        if (!ext_type->aliases) {
            sema_module_err(module, info->ext.of, "only type aliases can be extended");
            return;
        }
        if (keymap_insert((*vec_top(ext_type->aliases))->decls_map, info->name, decl)) {
            sema_module_err(module, info->name, "`$S` already declared in type alias", info->name);
            return;
        }
    } else {
        sema_module_push_decl(module, info->name, decl);
    }
}
