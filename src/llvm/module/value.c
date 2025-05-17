#include "value.h"
#include "sema/module/api/value.h"
#include "llvm/module/module.h"
#include "llvm/module/nodes/type.h"
#include <llvm-c/Core.h>

LLVMValueRef llvm_get_value(LlvmModule *module, LLVMValueRef source, size_t idx, SemaValue *value) {
    SemaType *type = sema_value_is_runtime(value);
    assert(type);
    if (sema_value_is_var(value)) {
        LLVMValueRef indices[2] = {
            LLVMConstInt(LLVMInt32Type(), 0, false),
            LLVMConstInt(LLVMInt32Type(), idx, false),
        };
        return LLVMBuildGEP2(module->builder, llvm_type(module, type), source, indices, 2, "");
    } else {
        return LLVMBuildExtractValue(module->builder, source, idx, "");
    }
}

LLVMValueRef llvm_opt_load(LlvmModule *module, LLVMValueRef source, SemaValue *value) {
    if (sema_value_is_var(value)) {
        return LLVMBuildLoad2(module->builder, llvm_type(module, sema_value_is_runtime(value)),
            source, "");
    }
    return source;
}
