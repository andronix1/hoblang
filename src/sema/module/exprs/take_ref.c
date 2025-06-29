#include "take_ref.h"
#include "ast/expr.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_take_ref(SemaModule *module, AstExpr *inner, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, inner,
        sema_expr_ctx_new(ctx.output, NULL)));
    if (runtime->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, inner->slice, "expression is not variable, so pointer cannot be taken");
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_take_ref(
        sema_module_expr_emit_runtime(module, runtime, ctx.output)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        sema_type_new_pointer(module->mempool, runtime->type), step_id);
}


