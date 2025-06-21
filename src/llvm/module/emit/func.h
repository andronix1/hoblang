#pragma once

#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMValueRef llvm_setup_func(LlvmModule *module, HirFuncId id);
void llvm_emit_func(LlvmModule *module, HirFuncId id, LLVMValueRef value);
