#pragma once

#include "hir/api/mut.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMTypeRef llvm_type_wrap_mutability(HirMutability mutability, LLVMTypeRef type);
LLVMValueRef llvm_value_get(LlvmModule *module, HirMutability mutability, LLVMTypeRef type, LLVMValueRef value);
