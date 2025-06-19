#include "api.h"
#include "core/log.h"
#include "core/mempool.h"
#include "hir/api/hir.h"
#include "hir/decl.h"
#include "hir/hir.h"
#include "module.h"
#include "llvm/module/emit/extern.h"
#include "llvm/module/emit/func.h"
#include "llvm/module/emit/var.h"
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <stdio.h>
#include <stdlib.h>

static bool llvm_setup(LlvmModule *module) {
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeTarget();
    LLVMTargetRef target = LLVMGetFirstTarget();
    if (!target) {
        logln("error: failed to initialize llvm target");
        return false;
    }
    char *features = LLVMGetHostCPUFeatures();
    char *cpu = LLVMGetHostCPUName();
    char *triple = LLVMGetDefaultTargetTriple();
    module->machine = LLVMCreateTargetMachine(target, triple, cpu, features, LLVMCodeGenLevelNone, LLVMRelocDefault, LLVMCodeModelDefault);
    free(features); free(cpu); free(triple);
    // module->state = llvm_state(NULL, NULL, NULL);
    module->context = LLVMContextCreate();
    module->module = LLVMModuleCreateWithNameInContext("main", module->context);
    module->builder = LLVMCreateBuilderInContext(module->context);
    return true;
}

LlvmModule *llvm_module_new() {
    LlvmModule *result = malloc(sizeof(LlvmModule));
    result->mempool = mempool_new(1024);
    if (!llvm_setup(result)) {
        llvm_module_free(result);
        return NULL;
    }
    return result;
}

void llvm_module_emit(LlvmModule *module, Hir *hir) {
    module->hir = hir;
    module->decls = vec_new_in(module->mempool, LLVMValueRef);
    vec_reserve(module->decls, vec_len(hir_get_decls(hir)));
    const HirDeclInfo *decls = hir_get_decls(module->hir);
    for (size_t i = 0; i < vec_len(decls); i++) {
        const HirDeclInfo *info = &decls[i];
        switch (info->kind) {
            case HIR_DECL_FUNC: llvm_setup_func(module, info->func, i); break;
            case HIR_DECL_EXTERN: llvm_setup_extern(module, info->external, i); break;
            case HIR_DECL_VAR: llvm_setup_var(module, info->var, i); break;
        }
    }
    for (size_t i = 0; i < vec_len(decls); i++) {
        const HirDeclInfo *info = &decls[i];
        switch (info->kind) {
            case HIR_DECL_FUNC: llvm_emit_func(module, info->func, i); break;
            case HIR_DECL_EXTERN: break;
            case HIR_DECL_VAR: llvm_init_var(module, info->var, i); break;
        }
    }
}

bool llvm_module_write_obj(LlvmModule *module, const char *output) {
    if (LLVMVerifyModule(module->module, LLVMAbortProcessAction | LLVMPrintMessageAction | LLVMReturnStatusAction, NULL)) {
        return false;
	}
	char *error;
	if (LLVMTargetMachineEmitToFile(module->machine, module->module, output, LLVMObjectFile, &error) == 1) {
		logln("error: failed to emit to file: %s", error);
		return false;
	}
	return true;
}

bool llvm_module_write_ir(LlvmModule *module, const char *output) {
	char *error;
	if (LLVMPrintModuleToFile(module->module, output, &error) == 1) {
		logln("error: failed to emit to file: %s", error);
		return false;
	}
	return true;
}

void llvm_module_free(LlvmModule *module) {
    LLVMDisposeBuilder(module->builder);
    LLVMDisposeTargetMachine(module->machine);
    LLVMContextDispose(module->context);
    LLVMShutdown();
    mempool_free(module->mempool);
    free(module);
}
