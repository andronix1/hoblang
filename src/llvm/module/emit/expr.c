#include "expr.h"
#include "core/assert.h"
#include "core/attributes.h"
#include "ir/api/ir.h"
#include "ir/stmt/expr.h"
#include "llvm/module/module.h"
#include <alloca.h>
#include <llvm-c/Core.h>

static LLVMValueRef llvm_emit_expr_step(LlvmModule *module, IrExprStep *step, bool load UNUSED) {
    switch (step->kind) {
        case IR_EXPR_STEP_INT:
            return LLVMConstInt(
                module->types[step->integer.type],
                step->integer.value,
                ir_type_int_is_signed(module->ir, step->integer.type)
            );
        case IR_EXPR_STEP_REAL:
        case IR_EXPR_STEP_BINOP:
        case IR_EXPR_STEP_STRUCT_FIELD:
        case IR_EXPR_STEP_GET_LOCAL:
            TODO;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_expr(LlvmModule *module, IrExpr *expr, bool load) {
    LLVMValueRef *values = alloca(sizeof(LLVMValueRef) * vec_len(expr->steps));
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        values[i] = llvm_emit_expr_step(module,
            &expr->steps[i],
            i != vec_len(expr->steps) - 1 || load
        );
    }
    return values[vec_len(expr->steps) - 1];
}

