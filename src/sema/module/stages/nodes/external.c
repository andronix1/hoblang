#include "external.h"
#include "core/null.h"
#include "core/assert.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stages/nodes/fun_info.h"
#include "sema/module/value.h"

bool sema_module_stage_fill_external(SemaModule *module, AstExternalDecl *external) {
    switch (external->kind) {
        case AST_EXTERNAL_DECL_FUN: {
            if (external->fun->generic) {
                sema_module_err(module, external->fun->name, "extern functions cannot be generic");
            }
            NOT_NULL(sema_func_info_setup(module, external->fun));
            SemaType *type = NOT_NULL(sema_func_info_type(module, external->fun));
            HirDeclId decl_id = hir_add_decl(module->hir);
            HirExternId extern_id = hir_add_extern(module->hir, opt_slice_unwrap_or(external->alias, external->fun->name),
                hir_extern_info_new(HIR_EXTERN_FUNC, sema_type_to_hir(module, type)));
            hir_init_decl_extern(module->hir, decl_id, extern_id);
            sema_module_push_fun_info_decl(module, external->fun, sema_value_new_runtime_global(module->mempool,
                SEMA_RUNTIME_FINAL, type, decl_id));
            return true;
        }
        case AST_EXTERNAL_DECL_VALUE: {
            assert(external->value->kind == AST_VALUE_DECL_VAR);
            if (!external->value->explicit_type) {
                sema_module_err(module, external->value->name, "extern value type must be specified");
                return false;
            }
            SemaType *type = NOT_NULL(sema_module_type(module, external->value->explicit_type));
            HirExternId extern_id = hir_add_extern(module->hir, opt_slice_unwrap_or(external->alias, external->fun->name),
                hir_extern_info_new(HIR_EXTERN_VAR, sema_type_to_hir(module, type)));
            HirDeclId decl_id = hir_add_decl(module->hir);
            hir_init_decl_extern(module->hir, decl_id, extern_id);
            sema_module_push_decl(module, external->value->name, sema_decl_new(module->mempool,
                external->value->is_public ? NULL : module, sema_value_new_runtime_global(module->mempool, 
                    SEMA_RUNTIME_VAR, type, decl_id)));
            return true;
        }
    }
    UNREACHABLE;
}
