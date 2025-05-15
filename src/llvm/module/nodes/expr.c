#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "sema/module/api/value.h"
#include "llvm/module/module.h"
#include "llvm/module/nodes/path.h"
#include "llvm/module/nodes/type.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_expr(LlvmModule *module, AstExpr *expr) {
    switch (expr->kind) {
        case AST_EXPR_PATH:
            return llvm_emit_path(module, expr->path);
        case AST_EXPR_INTEGER: {
            SemaType *type = sema_value_is_runtime(expr->sema.value);
            return LLVMConstInt(llvm_type(module, type), expr->integer, false);
        }
        case AST_EXPR_CALL: TODO;
        case AST_EXPR_SCOPE: return llvm_emit_expr(module, expr);
        case AST_EXPR_BINOP: {
            LLVMValueRef left = llvm_emit_expr_get(module, expr->binop.left);
            LLVMValueRef right = llvm_emit_expr_get(module, expr->binop.right);
            switch (expr->binop.kind) {
                case AST_BINOP_ADD: return LLVMBuildAdd(module->builder, left, right, "");
                case AST_BINOP_SUBTRACT: return LLVMBuildSub(module->builder, left, right, "");
                case AST_BINOP_MULTIPLY: return LLVMBuildMul(module->builder, left, right, "");
                case AST_BINOP_DIVIDE: return LLVMBuildUDiv(module->builder, left, right, "");
            }
            UNREACHABLE;
        }
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_expr_get(LlvmModule *module, AstExpr *expr) {
    LLVMValueRef value = llvm_emit_expr(module, expr);
    SemaType *type = sema_value_is_var(expr->sema.value);
    if (type) {
        return LLVMBuildLoad2(module->builder, llvm_type(module, type), value, "");
    }
    return value;
}
