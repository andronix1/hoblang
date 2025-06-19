#include "func.h"
#include "hir/api/hir.h"
#include "hir/api/code.h"
#include "hir/hir.h"
#include "llvm/module/emit/code.h"
#include "llvm/module/module.h"
#include "llvm/module/types.h"
#include <llvm-c/Core.h>

void llvm_setup_func(LlvmModule *module, HirFuncId id, HirDeclId decl) {
    const HirFuncInfo *info = hir_get_func_info(module->hir, id);
    module->decls[decl] = LLVMAddFunction(module->module, 
        mempool_opt_slice_to_cstr_or(module->mempool, info->global_name, ""),
        llvm_function_type(module, info->type));
}

void llvm_emit_func(LlvmModule *module, HirFuncId id, HirDeclId decl) {
    LLVMValueRef func = module->decls[decl];
    const HirFuncInfo *info = hir_get_func_info(module->hir, id);

    llvm_func_ctx_set(module, id, func);
    HirType *func_type = hir_resolve_simple_type(module->hir, info->type);
    assert(func_type->kind == HIR_TYPE_FUNCTION);
    for (size_t i = 0; i < vec_len(func_type->function.args); i++) {
        LLVMValueRef value = LLVMGetParam(func, i);
        HirLocalId local_id = hir_get_func_arg_local(module->hir, id, i);
        if (info->locals[local_id].mutability == HIR_MUTABLE) {
            LLVMValueRef new_value = llvm_alloca(module, llvm_runtime_type(module, func_type->function.args[i]));
            LLVMBuildStore(module->builder, value, new_value);
            value = new_value;
        }
        module->func.locals[local_id] = value;
    }
    llvm_emit_code(module, info->code);
    if (!info->code->breaks) {
        LLVMBuildRetVoid(module->builder);
    }
}
