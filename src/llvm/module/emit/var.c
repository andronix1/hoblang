#include "var.h"
#include "llvm/module/emit/expr.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"
#include <llvm-c/Core.h>

void llvm_setup_var(LlvmModule *module, HirVarId id, HirDeclId decl_id) {
    const HirVarInfo *info = hir_get_var_info(module->hir, id);
    LLVMValueRef value = LLVMAddGlobal(module->module, llvm_runtime_type(module, info->type),
        mempool_opt_slice_to_cstr_or(module->mempool, info->global_name, ""));
    module->decls[decl_id] = value;
}

void llvm_init_var(LlvmModule *module, HirVarId id, HirDeclId decl_id) {
    const HirVarInfo *info = hir_get_var_info(module->hir, id);
    if (info->initialized) {
        LLVMSetInitializer(module->decls[decl_id], llvm_emit_const(module, &info->initializer));
    }
}
