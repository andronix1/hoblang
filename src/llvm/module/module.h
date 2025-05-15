#pragma once

#include "core/mempool.h"
#include "sema/module/api.h"
#include "llvm/module/api.h"
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

typedef struct {
    LLVMValueRef func;
    LLVMBasicBlockRef code, defs;
} LlvmState;

typedef struct LlvmModule {
    SemaModule *sema;
    Mempool *mempool;

    LLVMTargetMachineRef machine;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
    LlvmState state;
} LlvmModule;

LlvmState llvm_switch_state(LlvmModule *module, LlvmState state);

static inline LlvmState llvm_state_new(LLVMValueRef func, LLVMBasicBlockRef code, LLVMBasicBlockRef defs) {
    LlvmState result = {
        .func = func,
        .code = code,
        .defs = defs
    };
    return result;
}
