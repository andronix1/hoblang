#pragma once

#include "sema/module/api/decl_handle.h"
#include "sema/module/type.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMTypeRef llvm_type(LlvmModule *module, SemaType *type);
LLVMTypeRef llvm_decl_type(LlvmModule *module, SemaDeclHandle *handle);
