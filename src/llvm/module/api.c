#include "api.h"
#include "ast/api/node.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api.h"
#include "module.h"
#include "llvm/module/nodes/node.h"
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
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

LlvmModule *llvm_module_new(SemaModule *sema) {
    LlvmModule *result = malloc(sizeof(LlvmModule));
    result->sema = sema;
    result->state = llvm_state_new(NULL, NULL, NULL);
    result->mempool = mempool_new(1024);
    if (!llvm_setup(result)) {
        llvm_module_free(result);
        return NULL;
    }
    return result;
}

void llvm_module_read(LlvmModule *module) {
    sema_module_read(module->sema);
    sema_module_analyze(module->sema);
    AstNode **nodes = sema_module_nodes(module->sema);
    for (size_t i = 0; i < vec_len(nodes); i++) {
        llvm_module_read_node(module, nodes[i]);
    }
}

void llvm_module_emit(LlvmModule *module) {
    AstNode **nodes = sema_module_nodes(module->sema);
    for (size_t i = 0; i < vec_len(nodes); i++) {
        llvm_module_emit_node(module, nodes[i]);
    }
}

bool llvm_module_write_obj(LlvmModule *module, const char *output) {
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
    sema_module_free(module->sema);
    free(module);
}
