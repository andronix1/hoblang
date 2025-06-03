#pragma once

#include "llvm/module/api.h"

void llvm_emit_func_body(LlvmModule *module, IrFuncId id);
