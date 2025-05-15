#pragma once

#include "ast/api/expr.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_expr(LlvmModule *module, AstExpr *expr);
LLVMValueRef llvm_emit_expr_get(LlvmModule *module, AstExpr *expr);
