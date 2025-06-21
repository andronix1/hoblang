#pragma once

#include "llvm/module/api.h"

void llvm_setup_gen(LlvmModule *module, HirGenScopeId id);
void llvm_emit_gen(LlvmModule *module, HirGenScopeId id);
