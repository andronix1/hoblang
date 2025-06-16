#include "binop.h"
#include "core/assert.h"
#include "core/null.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/expr/binop.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

inline bool sema_type_to_ir_number_info(SemaType *type, IrNumberInfo *output) {
    if (type->kind == SEMA_TYPE_INT) {
        *output = ir_number_info_new_int(type->integer.is_signed, sema_type_int_size_to_ir(type->integer.size));
        return true;
    }
    if (type->kind == SEMA_TYPE_FLOAT) {
        *output = ir_number_info_new_float(sema_type_float_size_to_ir(type->float_size));
        return true;
    }
    return false;
}

static SemaValue *sema_binop_order(
    SemaModule *module,
    IrBinopOrderKind kind,
    Slice where,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    if (!sema_type_to_ir_number_info(type, &info)) {
        sema_module_err(module, where, "this binop for number types only");
        return NULL;
    }
    size_t step_id = sema_expr_output_push_step(output, ir_expr_step_new_binop(
        ir_expr_binop_new_order(lss, rss, kind, info)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        sema_type_new_bool(module), step_id);
}

static SemaValue *sema_binop_compare(
    SemaModule *module,
    IrBinopCompareKind kind,
    Slice where,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    size_t step_id;
    if (sema_type_to_ir_number_info(type, &info)) {
        step_id = sema_expr_output_push_step(output, ir_expr_step_new_binop(
            ir_expr_binop_new_compare_number(lss, rss, kind, info)));
    } else if (type->kind == SEMA_TYPE_BOOL) {
        step_id = sema_expr_output_push_step(output, ir_expr_step_new_binop(
            ir_expr_binop_new_compare_bool(lss, rss, kind)));
    } else {
        sema_module_err(module, where, "this binop can be applied for bool or number");
        return NULL;
    }
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, sema_type_new_bool(module), step_id);
}

static SemaValue *sema_binop_arithm(
    SemaModule *module,
    IrBinopArithmeticKind kind,
    Slice where,
    SemaType *type,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    IrNumberInfo info;
    if (!sema_type_to_ir_number_info(type, &info)) {
        sema_module_err(module, where, "this binop for number types only");
        return NULL;
    }
    size_t step_id = sema_expr_output_push_step(output, ir_expr_step_new_binop(ir_expr_binop_new_arithmetic(
        lss, rss, kind, info)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
}

static SemaValue *sema_binop_int(
    SemaModule *module,
    IrBinopIntKind kind,
    SemaType *type,
    Slice pos,
    size_t lss, size_t rss,
    SemaExprOutput *output
) {
    if (type->kind != SEMA_TYPE_INT) {
        sema_module_err(module, pos, "this binop can be applied for integer operands only");
    }
    size_t step_id = sema_expr_output_push_step(output, 
        ir_expr_step_new_binop(ir_expr_binop_new_integer(lss, rss, kind)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
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
        sema_module_err(module, pos, "this binop can be applied for boolean operands only");
    }
    size_t step_id = sema_expr_output_push_step(output, 
        ir_expr_step_new_binop(ir_expr_binop_new_boolean(lss, rss, kind)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
}

SemaValue *sema_module_append_expr_binop(SemaModule *module, SemaType *type, size_t lss, size_t rss, AstBinopKind *kind, SemaExprOutput *output) {
    type = sema_type_resolve(type);
    switch (kind->kind) {
        case AST_BINOP_ADD: return sema_binop_arithm(module, IR_BINOP_ARITHM_ADD, kind->slice, type, lss, rss, output);
        case AST_BINOP_SUBTRACT: return sema_binop_arithm(module, IR_BINOP_ARITHM_SUB, kind->slice, type, lss, rss, output);
        case AST_BINOP_MULTIPLY: return sema_binop_arithm(module, IR_BINOP_ARITHM_MUL, kind->slice, type, lss, rss, output);
        case AST_BINOP_DIVIDE: return sema_binop_arithm(module, IR_BINOP_ARITHM_DIV, kind->slice, type, lss, rss, output);

        case AST_BINOP_EQUALS: return sema_binop_compare(module, IR_COMPARE_EQ, kind->slice, type, lss, rss, output);
        case AST_BINOP_NOT_EQUALS: return sema_binop_compare(module, IR_COMPARE_NE, kind->slice, type, lss, rss, output);

        case AST_BINOP_LESS: return sema_binop_order(module, IR_BINOP_ORDER_LT, kind->slice, type, lss, rss, output);
        case AST_BINOP_GREATER: return sema_binop_order(module, IR_BINOP_ORDER_GT, kind->slice, type, lss, rss, output);
        case AST_BINOP_LESS_EQ: return sema_binop_order(module, IR_BINOP_ORDER_LE, kind->slice, type, lss, rss, output);
        case AST_BINOP_GREATER_EQ: return sema_binop_order(module, IR_BINOP_ORDER_GE, kind->slice, type, lss, rss, output);

        case AST_BINOP_OR: return sema_binop_bool(module, IR_BINOP_BOOL_OR, type, kind->slice, lss, rss, output);
        case AST_BINOP_AND: return sema_binop_bool(module, IR_BINOP_BOOL_AND, type, kind->slice, lss, rss, output);

        case AST_BINOP_MOD: return sema_binop_int(module, IR_BINOP_INT_MOD, type, kind->slice, lss, rss, output);
        case AST_BINOP_BITOR: return sema_binop_int(module, IR_BINOP_INT_BITOR, type, kind->slice, lss, rss, output);
        case AST_BINOP_BITAND: return sema_binop_int(module, IR_BINOP_INT_BITAND, type, kind->slice, lss, rss, output);
        case AST_BINOP_SHL: return sema_binop_int(module, IR_BINOP_INT_SHL, type, kind->slice, lss, rss, output);
        case AST_BINOP_SHR: return sema_binop_int(module, IR_BINOP_INT_SHR, type, kind->slice, lss, rss, output);
    }
    UNREACHABLE;
}

SemaValue *sema_module_emit_expr_binop(SemaModule *module, AstBinop *binop, SemaExprCtx ctx) {
    SemaValueRuntime *ls = NOT_NULL(sema_module_emit_runtime_expr(module, binop->left,
        sema_expr_ctx_new(ctx.output, ctx.expectation)));
    size_t lss = sema_module_expr_emit_runtime(module->mempool, ls, ctx.output);
    if (binop->kind.kind == AST_BINOP_AND) {
        sema_expr_output_push_step(ctx.output, ir_expr_step_new_bool_skip(lss, false, false));
    } else if (binop->kind.kind == AST_BINOP_OR) {
        sema_expr_output_push_step(ctx.output, ir_expr_step_new_bool_skip(lss, true, true));
    }
    SemaValueRuntime *rs = NOT_NULL(sema_module_emit_runtime_expr(module, binop->right,
        sema_expr_ctx_new(ctx.output, ls->type)));
    size_t rss = sema_module_expr_emit_runtime(module->mempool, rs, ctx.output);
    if (!sema_type_eq(rs->type, ls->type)) {
        sema_module_err(module, binop->kind.slice, "binop can be applied to equal types only, but $t != $t", ls->type, rs->type);
        return NULL;
    }
    SemaType *type = rs->type;
    return sema_module_append_expr_binop(module, type, lss, rss, &binop->kind, ctx.output);
}


