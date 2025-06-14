#include "stmts.h"
#include "core/assert.h"
#include "core/vec.h"
#include "ir/api/ir.h"
#include "ir/ir.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "ir/type/type.h"
#include <alloca.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    Ir *ir;
    IrFuncId func;
} IrStmtCtx;

static void ir_fill_stmts_code(IrStmtCtx *ctx, IrCode *code);

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
        case IR_BINOP_BOOL:
            return ir_add_simple_type(ir, ir_type_new_bool());
        case IR_BINOP_INT:
            return types[binop->ls];
    }
    UNREACHABLE;
}

static inline IrTypeId ir_get_expr_step_type(IrStmtCtx *ctx, IrTypeId *types, IrExprStep *step) {
    Ir *ir = ctx->ir;
    switch (step->kind) {
        case IR_EXPR_STEP_CALL: {
            IrTypeId callable_id = ir_type_record_resolve_simple(ir, types[step->call.callable]);
            IrType *type = &ir->types[callable_id].simple;
            assert(type->kind == IR_TYPE_FUNCTION);
            return type->function.returns;
        }
        case IR_EXPR_STEP_INT: return step->integer.type;
        case IR_EXPR_STEP_REAL: return step->real.type;
        case IR_EXPR_STEP_BINOP: return ir_get_binop_type(ir, types, &step->binop);
        case IR_EXPR_STEP_GET_DECL: return ir->decls[step->decl_id].type;
        case IR_EXPR_STEP_TAKE_REF: return ir_add_simple_type(ir, ir_type_new_pointer(types[step->ref_step]));
        case IR_EXPR_STEP_DEREF: {
            IrTypeId source_id = ir_type_record_resolve_simple(ir, types[step->deref_step]);
            IrType *type = &ir->types[source_id].simple;
            assert(type->kind == IR_TYPE_POINTER);
            return type->pointer_to;
        }
        case IR_EXPR_STEP_GET_LOCAL: return ir->funcs[ctx->func].locals[step->local_id].type;
        case IR_EXPR_STEP_CAST_INT: return step->cast_int.dest;
        case IR_EXPR_STEP_BUILD_STRUCT: return step->build_struct.type;
        case IR_EXPR_STEP_STRUCT_FIELD: {
            IrTypeId source_id = ir_type_record_resolve_simple(ir, types[step->struct_field.step]);
            IrType *type = &ir->types[source_id].simple;
            assert(type->kind == IR_TYPE_STRUCT);
            return type->structure.fields[step->struct_field.idx];
        }
        case IR_EXPR_STEP_BOOL:
        case IR_EXPR_STEP_BOOL_SKIP:
        case IR_EXPR_STEP_NOT:
            return ir_add_simple_type(ir, ir_type_new_bool());
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

static inline IrCodeFlow _ir_fill_stmt_cond_jmp(IrStmtCtx *ctx, IrStmtCondJmp *cond_jmp) {
    bool passed = cond_jmp->else_code == NULL;
    size_t count = vec_len(cond_jmp->conds) + 1;
    IrCodeFlow *flows = passed ? NULL : alloca(sizeof(IrCodeFlow) * count);
    for (size_t i = 0; i < vec_len(cond_jmp->conds); i++) {
        IrStmtCondJmpBlock *block = &cond_jmp->conds[i];
        ir_fill_stmts_expr(ctx, &block->cond);
        ir_fill_stmts_code(ctx, block->code);
        if (!passed) {
            flows[i] = block->code->flow;
        }
    }
    if (cond_jmp->else_code) {
        ir_fill_stmts_code(ctx, cond_jmp->else_code);
        flows[count - 1] = cond_jmp->else_code->flow;
    }
    if (passed) {
        return IR_CODE_FLOW_PASSED;
    }
    bool was_return = false;
    bool was_break = false;
    bool was_unreachable = false;
    for (size_t i = 0; i < count; i++) {
        switch (flows[i]) {
            case IR_CODE_FLOW_PASSED: return IR_CODE_FLOW_PASSED;
            case IR_CODE_FLOW_LOOP_BREAK:
                was_break = true;
                break;
            case IR_CODE_FLOW_RETURN:
                was_return = true;
                break;
            case IR_CODE_FLOW_UNREACHABLE:
                was_unreachable = true;
                break;
        }
    }
    if (was_unreachable || (was_break && was_return)) {
        return IR_CODE_FLOW_UNREACHABLE;
    } else if (was_break) {
        return IR_CODE_FLOW_LOOP_BREAK;
    } else if (was_return) {
        return IR_CODE_FLOW_RETURN;
    }
    UNREACHABLE;
}

static IrCodeFlow ir_fill_stmt_cond_jmp(IrStmtCtx *ctx, IrStmtCondJmp *cond_jmp) {
    return cond_jmp->flow = _ir_fill_stmt_cond_jmp(ctx, cond_jmp);
}

static inline IrCodeFlow ir_fill_stmt(IrStmtCtx *ctx, IrStmt *stmt) {
    switch (stmt->kind) {
        case IR_STMT_EXPR:
            ir_fill_stmts_expr(ctx, &stmt->expr);
            return IR_CODE_FLOW_PASSED;
        case IR_STMT_RET:
            ir_fill_stmts_expr(ctx, &stmt->ret.value);
            return IR_CODE_FLOW_RETURN;
        case IR_STMT_STORE:
            ir_fill_stmts_expr(ctx, &stmt->store.lvalue);
            ir_fill_stmts_expr(ctx, &stmt->store.rvalue);
            return IR_CODE_FLOW_PASSED;
        case IR_STMT_INIT_FINAL:
            ir_fill_stmts_expr(ctx, &stmt->init_final.value);
            return IR_CODE_FLOW_PASSED;
        case IR_STMT_DECL_VAR:
            return IR_CODE_FLOW_PASSED;
        case IR_STMT_RET_VOID:
            return IR_CODE_FLOW_RETURN;
        case IR_STMT_COND_JMP:
            return ir_fill_stmt_cond_jmp(ctx, &stmt->cond_jmp);
        case IR_STMT_LOOP:
            ir_fill_stmts_code(ctx, stmt->loop.code);
            // TODO
            return IR_CODE_FLOW_PASSED;
        case IR_STMT_BREAK:
            return IR_CODE_FLOW_LOOP_BREAK;
        case IR_STMT_CONTINUE:
            return IR_CODE_FLOW_LOOP_BREAK;
    }
    UNREACHABLE;
}

static void ir_fill_stmts_code(IrStmtCtx *ctx, IrCode *code) {
    IrCodeFlow flow = IR_CODE_FLOW_PASSED;
    for (size_t i = 0; i < vec_len(code->stmts); i++) {
        flow = ir_fill_stmt(ctx, code->stmts[i]);
        assert(i == vec_len(code->stmts) - 1 || flow == IR_CODE_FLOW_PASSED);
    }
    code->flow = flow;
}

void ir_fill_stmts(Ir *ir) {
    for (size_t i = 0; i < vec_len(ir->funcs); i++) {
        IrStmtCtx ctx = ir_stmt_ctx_new(ir, i);
        IrFuncInfo *info = &ir->funcs[i];
        assert(info->func.code);
        ir_fill_stmts_code(&ctx, info->func.code);
    }
}
