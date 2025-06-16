#include "neg.h"
#include "core/null.h"
#include "ir/stmt/expr.h"
#include "sema/module/module.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/binop.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_neg(SemaModule *module, AstExpr *inner, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, inner, ctx));
    IrNumberInfo info;
    NOT_NULL(sema_type_to_ir_number_info(runtime->type, &info));
    size_t step_id = sema_expr_output_push_step(ctx.output, ir_expr_step_new_neg(
        sema_module_expr_emit_runtime(module->mempool, runtime, ctx.output), info));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, runtime->type, step_id);
}


