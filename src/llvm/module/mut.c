#include "mut.h"
#include "core/assert.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

LLVMTypeRef llvm_type_wrap_mutability(HirMutability mutability, LLVMTypeRef type) {
    switch (mutability) {
        case HIR_MUTABLE: return LLVMPointerType(type, 0);
        case HIR_IMMUTABLE: return type;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_value_get(LlvmModule *module, HirMutability mutability, LLVMTypeRef type, LLVMValueRef value) {
    switch (mutability) {
        case HIR_MUTABLE: return LLVMBuildLoad2(module->builder, type, value, "");
        case HIR_IMMUTABLE: return value;
    }
    UNREACHABLE;
}
