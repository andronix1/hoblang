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

static LLVMValueRef llvm_emit_expr_binop(LlvmModule *module, LLVMValueRef *values, IrBinop *binop) {
    LLVMValueRef ls = values[binop->ls];
    LLVMValueRef rs = values[binop->rs];
    switch (binop->kind) {
        case IR_BINOP_COMPARE:
            switch (binop->compare.kind) {
                case IR_COMPARE_EQ:
                    switch (binop->compare.val_kind) {
                        case IR_COMPARE_NUMBER:
                        case IR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntEQ, ls, rs, "");
                    }
                    UNREACHABLE;
                case IR_COMPARE_NE:
                    switch (binop->compare.val_kind) {
                        case IR_COMPARE_NUMBER:
                        case IR_COMPARE_BOOL:
                            return LLVMBuildICmp(module->builder, LLVMIntNE,
                                values[binop->ls], values[binop->rs], "");
                    }
                    UNREACHABLE;
            }
            UNREACHABLE;
        case IR_BINOP_ARITHMETIC:
            #define IR_BINOP_ARITHM_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case IR_NUMBER_INT: \
                        return INT(module->builder, ls, rs, ""); \
                    case IR_NUMBER_UINT: \
                        return UINT(module->builder, ls, rs, ""); \
                    case IR_NUMBER_FLOAT: \
                        return FLOAT(module->builder, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->arithm.kind) {
                case IR_BINOP_ARITHM_ADD:
                    IR_BINOP_ARITHM_OPT(LLVMBuildAdd, LLVMBuildAdd, LLVMBuildFAdd);
                case IR_BINOP_ARITHM_SUB:
                    IR_BINOP_ARITHM_OPT(LLVMBuildSub, LLVMBuildSub, LLVMBuildFSub);
                case IR_BINOP_ARITHM_MUL:
                    IR_BINOP_ARITHM_OPT(LLVMBuildMul, LLVMBuildMul, LLVMBuildFMul);
                case IR_BINOP_ARITHM_DIV:
                    IR_BINOP_ARITHM_OPT(LLVMBuildSDiv, LLVMBuildUDiv, LLVMBuildFDiv);
            }
            UNREACHABLE;
        case IR_BINOP_ORDER:
            #define IR_BINOP_ORDER_OPT(INT, UINT, FLOAT, ...) \
                switch (binop->arithm.number.kind) { \
                    case IR_NUMBER_INT: \
                        return LLVMBuildICmp(module->builder, INT, ls, rs, ""); \
                    case IR_NUMBER_UINT: \
                        return LLVMBuildICmp(module->builder, UINT, ls, rs, ""); \
                    case IR_NUMBER_FLOAT: \
                        return LLVMBuildFCmp(module->builder, FLOAT, ls, rs, ""); \
                } \
                UNREACHABLE;
            switch (binop->order.kind) {
                case IR_BINOP_ORDER_LT:
                    IR_BINOP_ORDER_OPT(LLVMIntSLT, LLVMIntULT, LLVMRealOLT);
                case IR_BINOP_ORDER_GT:
                    IR_BINOP_ORDER_OPT(LLVMIntSGT, LLVMIntUGT, LLVMRealOGT);
                case IR_BINOP_ORDER_LE:
                    IR_BINOP_ORDER_OPT(LLVMIntSLE, LLVMIntULE, LLVMRealOLE);
                case IR_BINOP_ORDER_GE:
                    IR_BINOP_ORDER_OPT(LLVMIntSGE, LLVMIntUGE, LLVMRealOGE);
                  break;
            }
            UNREACHABLE;
    }
    UNREACHABLE;
}

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
        case IR_EXPR_STEP_BINOP:
            return llvm_emit_expr_binop(module, values, &step->binop);
        case IR_EXPR_STEP_REAL:
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

