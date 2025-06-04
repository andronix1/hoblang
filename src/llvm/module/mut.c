#include "mut.h"
#include "core/assert.h"
#include "llvm/module/module.h"
#include <llvm-c/Core.h>

LLVMTypeRef llvm_type_wrap_mutability(IrMutability mutability, LLVMTypeRef type) {
    switch (mutability) {
        case IR_MUTABLE:
            return LLVMPointerType(type, 0);
        case IR_IMMUTABLE:
            return type;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_value_get(LlvmModule *module, IrMutability mutability, LLVMTypeRef type, LLVMValueRef value) {
    switch (mutability) {
        case IR_MUTABLE:
            return LLVMBuildLoad2(module->builder, type, value, "");
        case IR_IMMUTABLE:
            return value;
    }
    UNREACHABLE;
}
