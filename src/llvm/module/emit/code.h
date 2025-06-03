#pragma once

#include "llvm/module/api.h"

void llvm_emit_code(LlvmModule *module, IrCode *code);
