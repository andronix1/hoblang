#include "take_ref.h"
#include "ast/expr.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_analyze_expr_take_ref(SemaModule *module, AstExpr *inner, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_analyze_runtime_expr(module, inner,
        sema_expr_ctx_new(ctx.output, NULL)));
    if (runtime->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, inner->slice, "expression is not variable, so pointer cannot be taken");
    }
    vec_push(ctx.output->steps, ir_expr_step_new_take_ref(sema_module_expr_get_runtime(runtime, ctx.output)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        sema_type_new_pointer(module, runtime->type), vec_len(ctx.output->steps) - 1);
}


