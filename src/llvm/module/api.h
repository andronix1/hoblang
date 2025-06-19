#pragma once

#include "hir/api/hir.h"

typedef struct LlvmModule LlvmModule;

LlvmModule *llvm_module_new();
void llvm_module_emit(LlvmModule *module, Hir *hir);
bool llvm_module_write_obj(LlvmModule *module, const char *output);
bool llvm_module_write_ir(LlvmModule *module, const char *output);
void llvm_module_free(LlvmModule *module);
