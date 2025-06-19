#pragma once

#include "hir/api/expr.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_const(LlvmModule *module, const HirConst *constant);
LLVMValueRef llvm_emit_expr(LlvmModule *module, const HirExpr *expr, bool load);

