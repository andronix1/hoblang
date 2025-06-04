#pragma once

#include "ir/mut.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMTypeRef llvm_type_wrap_mutability(IrMutability mutability, LLVMTypeRef type);
LLVMValueRef llvm_value_get(LlvmModule *module, IrMutability mutability, LLVMTypeRef type, LLVMValueRef value);
