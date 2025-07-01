#include "fun_info.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "ast/node.h"
#include "sema/module/type/type.h"
#include "sema/module/value.h"

bool sema_func_info_setup(SemaModule *module, AstFunInfo *info) {
    info->ext.sema.type = NULL;
    info->ext.sema.generic = NULL;
    info->ext.sema.func_generic = NULL;
    if (info->ext.is) {
        SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl_from(module, module, info->ext.of));

        SemaType *ext_of = sema_value_is_type(decl->value);
        if (!ext_of) {
            SemaGeneric *generic = sema_value_is_generic(decl->value);
            if (generic && generic->kind == SEMA_GENERIC_TYPE) {
                // TODO: refactor this shit
                HirGenScopeId gen_scope = sema_module_add_gen_scope(module);
                SemaType **params = vec_new_in(module->mempool, SemaType*);
                for (size_t i = 0; i < vec_len(generic->gen_params); i++) {
                    HirGenParamId param = hir_add_gen_param(module->hir);
                    hir_gen_scope_add_param(module->hir, gen_scope, param);
                    assert(generic->gen_params[i]->kind == SEMA_TYPE_GENERIC);
                    vec_push(params, sema_type_new_gen_param(module->mempool, generic->gen_params[i]->generic_name, param));
                }
                SemaGeneric *func_generic = sema_generic_new_func(module->mempool, module, info->name, params, gen_scope);
                // ----------------
                ext_of = sema_value_is_type(sema_generate(generic, func_generic->gen_params));
                assert(ext_of);
                info->ext.sema.func_generic = func_generic;
                info->ext.sema.generic = generic;
            }
        }
        if (!ext_of) {
            sema_module_err(module, info->ext.of, "only types or generics can be extended");
            return NULL;
        }
        info->ext.sema.type = ext_of;
    }
    return true;
}

SemaType *sema_func_info_type(SemaModule *module, AstFunInfo *info) {
    SemaType **args = vec_new_in(module->mempool, SemaType*);
    vec_reserve(args, vec_len(info->args) + info->ext.is);

    SemaType *ext_of = info->ext.sema.type;
    if (ext_of) {
        vec_push(args, info->ext.by_ref ? sema_type_new_pointer(module->mempool, ext_of) : ext_of);
    }

    for (size_t i = 0; i < vec_len(info->args); i++) {
        vec_push(args, NOT_NULL(sema_module_type(module, info->args[i].type)));
    }

    SemaType *returns = NOT_NULL(sema_module_opt_type(module, info->returns));
    return sema_type_new_function(module->mempool, args, returns);
}

void sema_module_push_fun_info_decl(SemaModule *module, AstFunInfo *info, SemaValue *value) {
    SemaModule *source_module = info->is_public ? NULL : module;
    if (info->ext.is) {
        SemaType *ext_type = info->ext.sema.type;
        if (info->ext.sema.generic) {
            sema_generic_type_add_extension(module, info->ext.sema.generic, info->name,
                sema_alias_decl_new(value, source_module, info->ext.by_ref));
        } else {
            if (!ext_type->alias) {
                sema_module_err(module, info->ext.of, "only type aliases can be extended");
                return;
            }
            if (keymap_insert(ext_type->alias->decls_map, info->name,
                    sema_alias_decl_new(value, source_module, info->ext.by_ref))) {
                sema_module_err(module, info->name, "`$S` already declared in type alias", info->name);
                return;
            }
        }
    } else {
        sema_module_push_decl(module, info->name, sema_decl_new(module->mempool, source_module, value));
    }
}
