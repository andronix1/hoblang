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
    switch (binop.kind) {
        case AST_BINOP_ADD: return LLVMBuildAdd(module->builder, left, right, "");
        case AST_BINOP_SUBTRACT: return LLVMBuildSub(module->builder, left, right, "");
        case AST_BINOP_MULTIPLY: return LLVMBuildMul(module->builder, left, right, "");
        case AST_BINOP_DIVIDE:
            switch (binop.sema.arithmetic) {
                case SEMA_BINOP_ARITHMETIC_INT: return LLVMBuildSDiv(module->builder, left, right, "");
                case SEMA_BINOP_ARITHMETIC_UINT: return LLVMBuildUDiv(module->builder, left, right, "");
            }
            UNREACHABLE;

        #define BUILD_ANY_CMP(EINT, CMPINT, EUINT, CMPUINT) \
            switch (binop.sema.compare) { \
                case SEMA_BINOP_COMPARE_INT: return EINT(module->builder, CMPINT, left, right, ""); \
                case SEMA_BINOP_COMPARE_UINT: return EUINT(module->builder, CMPUINT, left, right, ""); \
            } \
            UNREACHABLE;

        case AST_BINOP_EQUALS: return LLVMBuildICmp(module->builder, LLVMIntEQ, left, right, "");
        case AST_BINOP_NOT_EQUALS: return LLVMBuildICmp(module->builder, LLVMIntNE, left, right, "");
        case AST_BINOP_LESS: BUILD_ANY_CMP(LLVMBuildICmp, LLVMIntSLT, LLVMBuildICmp, LLVMIntULT);
        case AST_BINOP_GREATER: BUILD_ANY_CMP(LLVMBuildICmp, LLVMIntSGT, LLVMBuildICmp, LLVMIntUGT);
        case AST_BINOP_LESS_EQ: BUILD_ANY_CMP(LLVMBuildICmp, LLVMIntSLE, LLVMBuildICmp, LLVMIntULE);
        case AST_BINOP_GREATER_EQ: BUILD_ANY_CMP(LLVMBuildICmp, LLVMIntSGE, LLVMBuildICmp, LLVMIntUGE);
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
            LLVMValueRef value = llvm_alloca(module, type);
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
