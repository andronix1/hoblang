#include "not.h"
#include "ast/expr.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_not(SemaModule *module, AstExpr *inner, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, inner,
        sema_expr_ctx_new(ctx.output, sema_type_new_bool(module))));
    if (runtime->type->kind != SEMA_TYPE_BOOL) {
        sema_module_err(module, inner->slice, "not operator can be applied to boolean expressions only, but it's of type $t", runtime->type);
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_not(
        sema_module_expr_emit_runtime(module, runtime, ctx.output)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, runtime->type, step_id);
}


