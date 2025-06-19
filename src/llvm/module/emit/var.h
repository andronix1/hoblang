#pragma once

#include "llvm/module/api.h"

void llvm_setup_var(LlvmModule *module, HirVarId id, HirDeclId decl_id);
void llvm_init_var(LlvmModule *module, HirVarId id, HirDeclId decl_id);
