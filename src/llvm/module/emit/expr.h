#pragma once

#include "ir/api/stmt/expr.h"
#include "llvm/module/api.h"
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_expr(LlvmModule *module, IrExpr *expr, bool load);

