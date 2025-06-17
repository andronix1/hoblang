#pragma once

#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMTypeRef llvm_function_type(LlvmModule *module, IrTypeId type_id);
void llvm_module_setup_types(LlvmModule *module);

