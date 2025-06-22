#include "external.h"
#include "core/null.h"
#include "core/assert.h"
#include "sema/module/module.h"
#include "sema/module/stages/nodes/fun_info.h"
#include "sema/module/value.h"

bool sema_module_stage_fill_external(SemaModule *module, AstExternalDecl *external) {
    switch (external->kind) {
        case AST_EXTERNAL_DECL_FUN: {
            if (external->fun->generic) {
                sema_module_err(module, external->fun->name, "extern functions cannot be generic");
            }
            SemaType *type = NOT_NULL(sema_func_info_type(module, external->fun));
            HirDeclId decl_id = hir_add_decl(module->hir);
            HirExternId extern_id = hir_add_extern(module->hir, opt_slice_unwrap_or(external->alias, external->fun->name),
                hir_extern_info_new(HIR_EXTERN_FUNC, sema_type_hir_id(type)));
            hir_init_decl_extern(module->hir, decl_id, extern_id);
            sema_module_push_fun_info_decl(module, external->fun, sema_value_new_runtime_global(module->mempool,
                SEMA_RUNTIME_FINAL, type, decl_id));
            return true;
        }
        case AST_EXTERNAL_DECL_VALUE:
            TODO;
    }
    UNREACHABLE;
}
