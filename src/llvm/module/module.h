#pragma once

#include "core/mempool.h"
#include "hir/api/hir.h"
#include "llvm/module/api.h"
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

typedef struct {
    LLVMBasicBlockRef begin;
    LLVMBasicBlockRef end;
} LlvmLoopInfo;

static inline LlvmLoopInfo llvm_loop_info_new(LLVMBasicBlockRef begin, LLVMBasicBlockRef end) {
    LlvmLoopInfo info = {
        .begin = begin,
        .end = end,
    };
    return info;
}

typedef struct {
    HirFuncId id;
    LLVMValueRef *locals;
    LlvmLoopInfo *loops;
    LLVMValueRef value;
    LLVMBasicBlockRef code, defs;
    bool in_loop;
} LlvmFuncCtx;

typedef struct {
    LLVMValueRef **funcs;
} LlvmGenScopeInfo;

typedef struct LlvmModule {
    Mempool *mempool;

    Hir *hir;
    LLVMValueRef *decls;
    LLVMTypeRef *gen_params;
    LlvmGenScopeInfo *gen_scopes;
    LlvmFuncCtx func;

    LLVMTargetMachineRef machine;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
} LlvmModule;

void llvm_func_ctx_set(LlvmModule *module, HirFuncId id, LLVMValueRef value);
LLVMValueRef llvm_alloca(LlvmModule *module, LLVMTypeRef type);

void llvm_module_set_block(LlvmModule *module, LLVMBasicBlockRef block);

bool llvm_module_begin_loop(LlvmModule *module);
void llvm_module_end_loop(LlvmModule *module);
