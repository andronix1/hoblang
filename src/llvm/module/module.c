#include "module.h"
#include <llvm-c/Core.h>

LlvmState llvm_switch_state(LlvmModule *llvm, LlvmState state) {
    LlvmState old_state = llvm->state;
    llvm->state = state;
    LLVMPositionBuilderAtEnd(llvm->builder, state.code);
    return old_state;
}

LLVMValueRef llvm_alloca(LlvmModule *module, LLVMTypeRef type) {
    return LLVMBuildAlloca(module->builder, type, "");
}
