#include "stmts.h"
#include "core/assert.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/ir.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include <alloca.h>
#include <stdio.h>
#include <string.h>

static inline IrTypeId ir_get_binop_type(Ir *ir, IrTypeId *types, IrBinop *binop) {
    switch (binop->kind) {
        case IR_BINOP_ARITHMETIC:
            return types[binop->ls];
        case IR_BINOP_ORDER:
        case IR_BINOP_COMPARE:
            return ir_add_simple_type(ir, ir_type_new_bool());
    }
    UNREACHABLE;
}

static inline IrTypeId ir_get_expr_step_type(Ir *ir, IrTypeId *types, IrExprStep *step) {
    switch (step->kind) {
        case IR_EXPR_STEP_CALL: {
            IrTypeId callable_id = ir_type_record_resolve_simple(ir,
                types[step->call.callable]);
            IrType *type = &ir->types[callable_id].simple;
            assert(type->kind == IR_TYPE_FUNCTION);
            return type->function.returns;
        }
        case IR_EXPR_STEP_INT:
            return step->integer.type;
        case IR_EXPR_STEP_REAL:
            return step->real.type;
        case IR_EXPR_STEP_BINOP:
            return ir_get_binop_type(ir, types, &step->binop);
        case IR_EXPR_STEP_GET_DECL:
            return ir->decls[step->decl_id].type;
        case IR_EXPR_STEP_STRUCT_FIELD:
        case IR_EXPR_STEP_GET_LOCAL:
            TODO;
    }
    UNREACHABLE;
}

static void ir_fill_stmts_expr(Ir *ir, IrExpr *expr) {
    IrTypeId *types = alloca(sizeof(IrTypeId) * vec_len(expr->steps));
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        types[i] = -1;
    }
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        IrExprStep *step = &expr->steps[i];
        types[i] = step->type = ir_get_expr_step_type(ir, types, step);
    }
}

static void ir_fill_stmts_code(Ir *ir, IrCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        IrStmt *stmt = code->stmts[i];
        switch (stmt->kind) {
            case IR_STMT_EXPR:
                ir_fill_stmts_expr(ir, &stmt->expr);
                break;
            case IR_STMT_RET:
                ir_fill_stmts_expr(ir, &stmt->ret.value);
                break;
            case IR_STMT_STORE:
            case IR_STMT_COND_JMP:
            case IR_STMT_RET_VOID:
            case IR_STMT_INIT_FINAL:
                TODO;
        }
    }
}

void ir_fill_stmts(Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->funcs); i++) {
        IrFuncInfo *info = &ir->funcs[i];
        assert(info->func.code);
        ir_fill_stmts_code(ir, info->func.code);
    }
}
