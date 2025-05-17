#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "llvm/module/module.h"
#include "llvm/module/nodes/path.h"
#include "llvm/module/nodes/type.h"
#include <alloca.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

LLVMValueRef llvm_emit_binop(LlvmModule *module, LLVMValueRef left, LLVMValueRef right, AstBinopKind binop) {
    switch (binop) {
        case AST_BINOP_ADD: return LLVMBuildAdd(module->builder, left, right, "");
        case AST_BINOP_SUBTRACT: return LLVMBuildSub(module->builder, left, right, "");
        case AST_BINOP_MULTIPLY: return LLVMBuildMul(module->builder, left, right, "");
        case AST_BINOP_DIVIDE: return LLVMBuildUDiv(module->builder, left, right, "");
    }
    UNREACHABLE;

}

LLVMValueRef llvm_emit_expr(LlvmModule *module, AstExpr *expr) {
    switch (expr->kind) {
        case AST_EXPR_PATH:
            return llvm_emit_path(module, expr->path);
        case AST_EXPR_INTEGER: {
            SemaType *type = sema_value_is_runtime(expr->sema.value);
            return LLVMConstInt(llvm_type(module, type), expr->integer, false);
        }
        case AST_EXPR_CALL: {
            size_t count = vec_len(expr->call.args);
            LLVMValueRef callable = llvm_emit_expr_get(module, expr->call.inner);
            SemaDeclHandle *ext = sema_value_is_ext(expr->call.inner->sema.value);
            size_t offset = ext != NULL;
            LLVMValueRef *args = alloca(sizeof(LLVMValueRef) * (count + offset));
            if (ext) {
                args[0] = ext->llvm.value;
            }
            for (size_t i = 0; i < count; i++) {
                args[i + offset] = llvm_emit_expr_get(module, expr->call.args[i]);
            }
            return LLVMBuildCall2(module->builder,
                llvm_type(module, sema_value_is_runtime(expr->call.inner->sema.value)),
                callable, args, count + offset, "");
        }
        case AST_EXPR_SCOPE: return llvm_emit_expr(module, expr);
        case AST_EXPR_BINOP: {
            LLVMValueRef left = llvm_emit_expr_get(module, expr->binop.left);
            LLVMValueRef right = llvm_emit_expr_get(module, expr->binop.right);
            return llvm_emit_binop(module, left, right, expr->binop.kind);
        }
        case AST_EXPR_STRING: {
            // TODO: slice
            LLVMValueRef value = LLVMAddGlobal(module->module, LLVMArrayType2(LLVMInt8Type(),
                expr->string.length), "");
            LLVMSetInitializer(value, LLVMConstString(expr->string.value,
                expr->string.length, true));
            return value;
        }
        case AST_EXPR_STRUCT: {
            // TODO: move in defs block
            LLVMTypeRef type = llvm_type(module,  sema_value_is_runtime(expr->sema.value));
            LLVMValueRef value = LLVMBuildAlloca(module->builder, type, "");
            for (size_t i = 0; i < vec_len(expr->structure.fields_map); i++) {
                keymap_at(expr->structure.fields_map, i, field);
                LLVMValueRef indices[2] = {
                    LLVMConstInt(LLVMInt32Type(), 0, false),
                    LLVMConstInt(LLVMInt32Type(), field->value.sema.field_idx, false),
                };
                LLVMBuildStore(module->builder, llvm_emit_expr_get(module, field->value.expr),
                    LLVMBuildGEP2(module->builder, type, value, indices, 2, ""));
            }
            return LLVMBuildLoad2(module->builder, type, value, "");
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
