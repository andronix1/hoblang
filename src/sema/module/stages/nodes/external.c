#include "external.h"
#include "core/null.h"
#include "core/assert.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stages/nodes/fun_info.h"
#include "sema/module/value.h"

bool sema_module_stage_fill_external(SemaModule *module, AstExternalDecl *external) {
    switch (external->kind) {
        case AST_EXTERNAL_DECL_FUN: {
            SemaType *type = NOT_NULL(sema_func_info_type(module, external->fun));
            IrDeclId decl_id = ir_add_decl(module->ir);
            ir_init_extern(module->ir, decl_id, ir_extern_new(IR_EXTERN_FUNC,
                external->has_alias ? external->alias : external->fun->name, sema_type_ir_id(type)));
            sema_module_push_fun_info_decl(module, external->fun, sema_decl_new(module->mempool,
                external->fun->is_public ? NULL : module,
                sema_value_new_runtime_global(module->mempool, SEMA_RUNTIME_FINAL, type, decl_id)));
            return true;
        }
        case AST_EXTERNAL_DECL_VALUE:
            TODO;
    }
    UNREACHABLE;
}
