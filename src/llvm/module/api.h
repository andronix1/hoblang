#pragma once

#include "hir/api/hir.h"

typedef struct LlvmModule LlvmModule;

typedef struct {
    bool release;
} LlvmModuleConfig;

static inline LlvmModuleConfig llvm_module_config(bool release) {
    LlvmModuleConfig config = { .release = release };
    return config;
}

LlvmModule *llvm_module_new(LlvmModuleConfig config);
void llvm_module_emit(LlvmModule *module, Hir *hir);
bool llvm_module_write_obj(LlvmModule *module, const char *output);
bool llvm_module_write_ir(LlvmModule *module, const char *output);
void llvm_module_free(LlvmModule *module);
