#pragma once

#include "sema/module/api/value.h"
#include "llvm/module/api.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

LLVMValueRef llvm_get_value(LlvmModule *module, LLVMValueRef source, size_t idx, SemaValue *value);
