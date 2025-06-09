#include "module.h"
#include <llvm-c/Core.h>
#include "core/mempool.h"
#include "ir/ir.h"

void llvm_func_ctx_set(LlvmModule *module, IrFuncId id, LLVMValueRef value) {
    module->func.id = id;
    module->func.value = value;
    module->func.locals = __mempool_alloc(module->mempool,
        sizeof(LLVMValueRef) * vec_len(module->ir->funcs[id].locals),
        alignof(LLVMValueRef)
    );
    module->func.loops = __mempool_alloc(module->mempool,
        sizeof(LlvmLoopInfo) * module->ir->funcs[id].loops_count,
        alignof(LlvmLoopInfo)
    );
}

LlvmState llvm_switch_state(LlvmModule *llvm, LlvmState state) {
    LlvmState old_state = llvm->state;
    llvm->state = state;
    LLVMPositionBuilderAtEnd(llvm->builder, state.code);
    return old_state;
}

LLVMValueRef llvm_alloca(LlvmModule *module, LLVMTypeRef type) {
    return LLVMBuildAlloca(module->builder, type, "");
}
