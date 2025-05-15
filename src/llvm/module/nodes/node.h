#pragma once

#include "llvm/module/api.h"

void llvm_module_read_node(LlvmModule *module, AstNode *node);
void llvm_module_emit_node(LlvmModule *module, AstNode *node);
