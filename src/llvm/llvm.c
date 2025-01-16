#include "llvm.h"

bool llvm_init(LlvmBackend *llvm) {
	llvm->builder = LLVMCreateBuilder();
	return true;	
}

LLVMTypeRef llvm_resolve_type(SemaType *type) {
	switch (type->type) {
		case SEMA_TYPE_PRIMITIVE:
			switch (type->primitive) {
				case PRIMITIVE_I8: case PRIMITIVE_U8: return LLVMInt8Type();
				case PRIMITIVE_I16: case PRIMITIVE_U16: return LLVMInt16Type();
				case PRIMITIVE_I32: case PRIMITIVE_U32: return LLVMInt32Type();
				case PRIMITIVE_I64: case PRIMITIVE_U64: return LLVMInt64Type();
				case PRIMITIVE_BOOL: return LLVMInt1Type();
				case PRIMITIVE_VOID: return LLVMVoidType();
			}
			assert(0, "invalid primitive {int}", type->primitive);
			return NULL;
		case SEMA_TYPE_FUNCTION: {
			LLVMTypeRef *params = malloc(sizeof(LLVMTypeRef) * vec_len(type->func.args));
			for (size_t i = 0; i < vec_len(type->func.args); i++) {
				params[i] = llvm_resolve_type(type->func.args[i].type.sema);
			}
			return LLVMFunctionType(llvm_resolve_type(type->func.returning), params, vec_len(type->func.args), false /* IsVarArg */);
		}
		case SEMA_TYPE_POINTER: {
			return LLVMPointerType(llvm_resolve_type(type->ptr_to), 0);
		}
	}
	assert(0, "invalid type {int}", type->type);
}

bool llvm_write_module_ir(LlvmBackend *llvm, char *output_path) {
	char *error;	
	if (LLVMPrintModuleToFile(llvm->module, output_path, &error) == 1) {
		hob_log(LOGE, "failed to emit to file: %s", error);
		return false;
	}
	return true;
}

bool llvm_write_module(LlvmBackend *llvm, char *output_path) {
	if (LLVMVerifyModule(llvm->module, LLVMAbortProcessAction | LLVMPrintMessageAction | LLVMReturnStatusAction, NULL)) {
		llvm_write_module_ir(llvm, "dump.ll");
		exit(1);
	}

	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();

	LLVMTargetRef target = LLVMGetFirstTarget();
	// const char *target_name = "aarch64";
	//LLVMTargetRef target = LLVMGetTargetFromName(target_name);
	if (!target) {
		hob_log(LOGE, "failed to initialize target");
		return false;
	}
	const char *features = LLVMGetHostCPUFeatures();
	const char *cpu = LLVMGetHostCPUName();
	const char *triple = LLVMGetDefaultTargetTriple();
	hob_log(LOGD, "target: {cstr} - {cstr}", LLVMGetTargetName(target), LLVMGetTargetDescription(target));
	hob_log(LOGD, "triple: {cstr}", triple);
	hob_log(LOGD, "with features {cstr}", features);
	hob_log(LOGD, "cpu: {cstr}", cpu);
	llvm->machine = LLVMCreateTargetMachine(target, triple, cpu, features, LLVMCodeGenLevelNone, LLVMRelocDefault, LLVMCodeModelDefault);
	if (!llvm->machine) {
		hob_log(LOGE, "failed to create target machine");
		return false;
	}
	char *error;
	if (LLVMTargetMachineEmitToFile(llvm->machine, llvm->module, output_path, LLVMObjectFile, &error) == 1) {
		hob_log(LOGE, "failed to emit to file: %s", error);
		return false;
	}
	return true;
}
