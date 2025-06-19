#include "module.h"
#include <llvm-c/Core.h>
#include "core/mempool.h"

void llvm_func_ctx_set(LlvmModule *module, HirFuncId id, LLVMValueRef func) {
    const HirFuncInfo *info = hir_get_func_info(module->hir, id);
    module->func.id = id;
    module->func.value = func;
    module->func.locals = __mempool_alloc(module->mempool,
        sizeof(LLVMValueRef) * vec_len(info->locals),
        alignof(LLVMValueRef)
    );
    module->func.loops = __mempool_alloc(module->mempool,
        sizeof(LlvmLoopInfo) * info->loops_count,
        alignof(LlvmLoopInfo)
    );
    module->func.in_loop = false;
    llvm_module_set_block(module, LLVMAppendBasicBlock(func, ""));
}

LLVMValueRef llvm_alloca(LlvmModule *module, LLVMTypeRef type) {
    LLVMPositionBuilderAtEnd(module->builder, module->func.defs);
    LLVMValueRef value = LLVMBuildAlloca(module->builder, type, "");
    LLVMPositionBuilderAtEnd(module->builder, module->func.code);
    return value;
}

void llvm_module_set_block(LlvmModule *module, LLVMBasicBlockRef block) {
    module->func.code = block;
    if (!module->func.in_loop) {
        module->func.defs = block;
    }
    LLVMPositionBuilderAtEnd(module->builder, block);
}
