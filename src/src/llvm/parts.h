#pragma once

#include "llvm.h"
#include "sema/type.h"
#include "sema/project.h"

void llvm_module(LlvmBackend *llvm, AstModule *module);
void llvm_body_break(LlvmBackend *llvm, AstBody *body);
bool llvm_body(LlvmBackend *llvm, AstBody *body);
LLVMValueRef llvm_sema_value(LlvmBackend *llvm, SemaScopeValueDecl *decl);
LLVMValueRef llvm_expr(LlvmBackend *llvm, AstExpr *expr);
LLVMValueRef llvm_func_call(LlvmBackend *llvm, AstFuncCall *func_call);
LLVMValueRef llvm_value(LlvmBackend *llvm, AstValue *value);
