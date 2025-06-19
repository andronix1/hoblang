#pragma once

#include "llvm/module/api.h"

void llvm_setup_func(LlvmModule *module, HirFuncId id, HirDeclId decl);
void llvm_emit_func(LlvmModule *module, HirFuncId id, HirDeclId decl);
