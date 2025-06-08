#include "binop.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/expr/binop.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

static bool sema_type_to_ir_number_info(
    SemaType *type,
    IrNumberInfo *output
) {
    if (type->kind == SEMA_TYPE_INT) {
        *output = ir_number_info_new_int(type->integer.is_signed, sema_type_int_size_to_ir(type->integer.size));
        return true;
    }
    return false;
}

static SemaValue *sema_binop_order(
    SemaModule *module,
    IrBinopOrderKind kind,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    size_t step_id = vec_len(output->steps);
    NOT_NULL(sema_type_to_ir_number_info(type, &info));
    vec_push(output->steps, ir_expr_step_new_binop(ir_expr_binop_new_order(lss, rss, kind, info)));
    return sema_value_new_runtime_expr_step(module->mempool, sema_type_new_bool(module), step_id);
}

static SemaValue *sema_binop_compare(
    SemaModule *module,
    IrBinopCompareKind kind,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    size_t step_id = vec_len(output->steps);
    if (sema_type_to_ir_number_info(type, &info)) {
        vec_push(output->steps, ir_expr_step_new_binop(ir_expr_binop_new_compare_number(
            lss, rss, kind, info)));
    } else if (type->kind == SEMA_TYPE_BOOL) {
        vec_push(output->steps, ir_expr_step_new_binop(ir_expr_binop_new_compare_bool(
            lss, rss, kind)));
    } else {
        return NULL;
    }
    return sema_value_new_runtime_expr_step(module->mempool, sema_type_new_bool(module), step_id);
}

static SemaValue *sema_binop_arithm(
    SemaModule *module,
    IrBinopArithmeticKind kind,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    NOT_NULL(sema_type_to_ir_number_info(type, &info));
    size_t step_id = vec_len(output->steps);
    vec_push(output->steps, ir_expr_step_new_binop(ir_expr_binop_new_arithmetic(
        lss, rss, kind, info)));
    return sema_value_new_runtime_expr_step(module->mempool, type, step_id);
}

static SemaValue *sema_binop_bool(
    SemaModule *module,
    IrBinopBoolKind kind,
    SemaType *type,
    Slice pos,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    if (type->kind != SEMA_TYPE_BOOL) {
        sema_module_err(module, pos, "this binop can be used for booleans only");
    }
    size_t step_id = vec_len(output->steps);
    vec_push(output->steps, ir_expr_step_new_binop(ir_expr_binop_new_boolean(
        lss, rss, kind)));
    return sema_value_new_runtime_expr_step(module->mempool, type, step_id);
}

SemaValue *sema_module_analyze_expr_binop(SemaModule *module, AstBinop *binop, SemaExprCtx ctx) {
    SemaValueRuntime *ls = NOT_NULL(sema_module_analyze_runtime_expr(module, binop->left, sema_expr_ctx_new(ctx.output, ctx.expectation)));
    size_t lss = sema_module_expr_get_runtime(ls, ctx.output);
    if (binop->kind.kind == AST_BINOP_AND) {
        vec_push(ctx.output->steps, ir_expr_step_new_bool_skip(lss, false, false));
    } else if (binop->kind.kind == AST_BINOP_OR) {
        vec_push(ctx.output->steps, ir_expr_step_new_bool_skip(lss, true, true));
    }
    SemaValueRuntime *rs = NOT_NULL(sema_module_analyze_runtime_expr(module, binop->right, sema_expr_ctx_new(ctx.output, ls->type)));
    size_t rss = sema_module_expr_get_runtime(rs, ctx.output);
    if (!sema_type_eq(rs->type, ls->type)) {
        sema_module_err(module, binop->kind.slice, "binop can be applied to equal types only, but $t != $t", ls->type, rs->type);
        return NULL;
    }
    SemaType *type = rs->type;
    switch (binop->kind.kind) {
        case AST_BINOP_ADD:
            return sema_binop_arithm(module, IR_BINOP_ARITHM_ADD, type, lss, rss, ctx.output);
        case AST_BINOP_SUBTRACT:
            return sema_binop_arithm(module, IR_BINOP_ARITHM_SUB, type, lss, rss, ctx.output);
        case AST_BINOP_MULTIPLY:
            return sema_binop_arithm(module, IR_BINOP_ARITHM_MUL, type, lss, rss, ctx.output);
        case AST_BINOP_DIVIDE:
            return sema_binop_arithm(module, IR_BINOP_ARITHM_DIV, type, lss, rss, ctx.output);

        case AST_BINOP_EQUALS:
            return sema_binop_compare(module, IR_COMPARE_EQ, type, lss, rss, ctx.output);
        case AST_BINOP_NOT_EQUALS:
            return sema_binop_compare(module, IR_COMPARE_NE, type, lss, rss, ctx.output);

        case AST_BINOP_LESS:
            return sema_binop_order(module, IR_BINOP_ORDER_LT, type, lss, rss, ctx.output);
        case AST_BINOP_GREATER:
            return sema_binop_order(module, IR_BINOP_ORDER_GT, type, lss, rss, ctx.output);
        case AST_BINOP_LESS_EQ:
            return sema_binop_order(module, IR_BINOP_ORDER_LE, type, lss, rss, ctx.output);
        case AST_BINOP_GREATER_EQ:
            return sema_binop_order(module, IR_BINOP_ORDER_GE, type, lss, rss, ctx.output);

        case AST_BINOP_OR:
            return sema_binop_bool(module, IR_BINOP_BOOL_OR, type, binop->kind.slice,
                lss, rss, ctx.output);
        case AST_BINOP_AND:
            return sema_binop_bool(module, IR_BINOP_BOOL_AND, type, binop->kind.slice,
                lss, rss, ctx.output);
    }
    UNREACHABLE;
}


