#include "llvm/private.h"
#include "llvm/impl.h"
#include "core/vec.h"
#include "ast/private/type.h"
#include "sema/type/private.h"

LLVMModuleRef llvm_current_module(LlvmBackend *llvm) {
    return llvm->module;
}

LLVMValueRef llvm_current_func(LlvmBackend *llvm) {
    return llvm->func;
}

void llvm_set_current_func(LlvmBackend *llvm, LLVMValueRef func) {
    llvm->func = func;
}

LLVMBuilderRef llvm_builder(LlvmBackend *llvm) {
    return llvm->builder;
}
