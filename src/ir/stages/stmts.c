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

typedef struct {
    Ir *ir;
    IrFuncId func;
} IrStmtCtx;

static inline IrStmtCtx ir_stmt_ctx_new(Ir *ir, IrFuncId func) {
    IrStmtCtx ctx = {
        .ir = ir,
        .func = func
    };
    return ctx;
}

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

static inline IrTypeId ir_get_expr_step_type(IrStmtCtx *ctx, IrTypeId *types, IrExprStep *step) {
    Ir *ir = ctx->ir;
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
        case IR_EXPR_STEP_GET_LOCAL:
            return ir->funcs[ctx->func].locals[step->local_id].type;
        case IR_EXPR_STEP_STRUCT_FIELD:
            TODO;
    }
    UNREACHABLE;
}

static void ir_fill_stmts_expr(IrStmtCtx *ctx, IrExpr *expr) {
    IrTypeId *types = alloca(sizeof(IrTypeId) * vec_len(expr->steps));
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        types[i] = -1;
    }
    for (size_t i = 0; i < vec_len(expr->steps); i++) {
        IrExprStep *step = &expr->steps[i];
        types[i] = step->type = ir_get_expr_step_type(ctx, types, step);
    }
}

static void ir_fill_stmts_code(IrStmtCtx *ctx, IrCode *code) {
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        IrStmt *stmt = code->stmts[i];
        switch (stmt->kind) {
            case IR_STMT_EXPR:
                ir_fill_stmts_expr(ctx, &stmt->expr);
                break;
            case IR_STMT_RET:
                ir_fill_stmts_expr(ctx, &stmt->ret.value);
                break;
            case IR_STMT_STORE:
                ir_fill_stmts_expr(ctx, &stmt->store.lvalue);
                ir_fill_stmts_expr(ctx, &stmt->store.rvalue);
                break;
            case IR_STMT_INIT_FINAL:
                ir_fill_stmts_expr(ctx, &stmt->init_final.value);
                break;
            case IR_STMT_DECL_VAR:
                break;
            case IR_STMT_COND_JMP:
            case IR_STMT_RET_VOID:
                TODO;
        }
    }
}

void ir_fill_stmts(Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->funcs); i++) {
        IrStmtCtx ctx = ir_stmt_ctx_new(ir, i);
        IrFuncInfo *info = &ir->funcs[i];
        assert(info->func.code);
        ir_fill_stmts_code(&ctx, info->func.code);
    }
}
