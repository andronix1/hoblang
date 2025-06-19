#pragma once

#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMTypeRef llvm_function_type(LlvmModule *module, HirTypeId type_id);
LLVMTypeRef llvm_runtime_type(LlvmModule *module, HirTypeId type_id);

