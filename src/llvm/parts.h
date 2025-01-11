#pragma once

#include "llvm.h"
#include "sema/type.h"

void llvm_module(LlvmBackend *llvm, AstModule *module);
void llvm_body(LlvmBackend *llvm, AstBody *body);
LLVMValueRef llvm_expr(LlvmBackend *llvm, AstExpr *expr);
LLVMValueRef llvm_func_call(LlvmBackend *llvm, AstFuncCall *func_call);
