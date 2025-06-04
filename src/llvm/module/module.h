#pragma once

#include "core/mempool.h"
#include "ir/api/ir.h"
#include "llvm/module/api.h"
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

typedef struct {
    LLVMValueRef func;
    LLVMBasicBlockRef code, defs;
} LlvmState;

typedef struct {
    IrFuncId id;
    LLVMValueRef *locals;
} LlvmFuncCtx;

void llvm_func_ctx_set(LlvmModule *module, IrFuncId id);

typedef struct LlvmModule {
    Mempool *mempool;

    Ir *ir;
    LLVMValueRef *decls;
    LLVMTypeRef *types;
    LlvmFuncCtx func;

    LLVMTargetMachineRef machine;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
    LlvmState state;
} LlvmModule;

LlvmState llvm_switch_state(LlvmModule *module, LlvmState state);
LLVMValueRef llvm_alloca(LlvmModule *module, LLVMTypeRef type);

static inline LlvmState llvm_state_new(LLVMValueRef func, LLVMBasicBlockRef code, LLVMBasicBlockRef defs) {
    LlvmState result = {
        .func = func,
        .code = code,
        .defs = defs
    };
    return result;
}
