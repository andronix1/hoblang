#include "expr.h"
#include "core/assert.h"
#include "ir/api/ir.h"
#include "ir/decls.h"
#include "ir/stmt/expr.h"
#include "llvm/module/module.h"
#include "llvm/module/mut.h"
#include "ir/ir.h"
#include <alloca.h>
#include <llvm-c/Core.h>
#include <stdio.h>

static LLVMValueRef llvm_emit_expr_step(LlvmModule *module, LLVMValueRef *values, IrExprStep *steps, size_t step_id, bool load) {
    IrExprStep *step = &steps[step_id];
    switch (step->kind) {
        case IR_EXPR_STEP_INT:
            return LLVMConstInt(
                module->types[step->integer.type],
                step->integer.value,
                ir_type_int_is_signed(module->ir, step->integer.type)
            );
        case IR_EXPR_STEP_CALL: {
            LLVMValueRef *args = alloca(sizeof(LLVMValueRef) * vec_len(step->call.args));
            for (size_t i = 0; i < vec_len(step->call.args); i++) {
                args[i] = values[step->call.args[i]];
            }
            return LLVMBuildCall2(
                module->builder,
                module->types[steps[step->call.callable].type],
                values[step->call.callable],
                args, vec_len(step->call.args),
                ""
            );
        }
        case IR_EXPR_STEP_GET_DECL: {
            IrDecl *decl = &module->ir->decls[step->decl_id];
            return load ? llvm_value_get(
                module,
                decl->mutability,
                module->types[decl->type],
                module->decls[step->decl_id]
            ) : module->decls[step->decl_id];
        }
        case IR_EXPR_STEP_GET_LOCAL: {
            IrFuncLocal *local = &module->ir->funcs[module->func.id].locals[step->local_id];
            return load ? llvm_value_get(
                module,
                local->mutability,
                module->types[local->type],
                module->func.locals[step->local_id]
            ) : module->func.locals[step->local_id];
        }
        case IR_EXPR_STEP_REAL:
        case IR_EXPR_STEP_BINOP:
        case IR_EXPR_STEP_STRUCT_FIELD:
            TODO;
    }
    UNREACHABLE;
}

LLVMValueRef llvm_emit_expr(LlvmModule *module, IrExpr *expr, bool load) {
    LLVMValueRef *values = alloca(sizeof(LLVMValueRef) * vec_len(expr->steps));
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        values[i] = llvm_emit_expr_step(module, values,
            expr->steps, i,
            i != vec_len(expr->steps) - 1 || load
        );
    }
    return values[vec_len(expr->steps) - 1];
}

