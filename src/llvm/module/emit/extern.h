#pragma once

#include "llvm/module/api.h"

void llvm_setup_extern(LlvmModule *module, HirExternId id, HirDeclId decl);
