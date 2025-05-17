#pragma once

#include "ast/api/expr.h"
#include "ast/expr.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_binop(LlvmModule *module, LLVMValueRef left, LLVMValueRef right, AstBinopKind binop);
LLVMValueRef llvm_emit_expr(LlvmModule *module, AstExpr *expr);
LLVMValueRef llvm_emit_expr_get(LlvmModule *module, AstExpr *expr);
