#pragma once

#include "ast/api/path.h"
#include "llvm/module/api.h"
#include <llvm-c/Core.h>

LLVMValueRef llvm_emit_path(LlvmModule *module, AstPath *path);
