#include "call.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/stmt/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/value.h"
#include <stdio.h>

SemaValue *sema_module_analyze_expr_call(SemaModule *module, AstCall *call, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_analyze_runtime_expr(module,
        call->inner, sema_expr_ctx_new(ctx.output, NULL))); 
    SemaType *type = runtime->type;
    if (type->kind != SEMA_TYPE_FUNCTION) {
        sema_module_err(module, call->inner->slice, "$t is not callable", type);
        return NULL;
    }
    if (vec_len(type->function.args) != vec_len(call->args)) {
        sema_module_err(module, call->inner->slice, "expected to $l arguments to be passed bu $l was passed", vec_len(type->function.args), vec_len(call->args));
        return NULL;
    }
    size_t callable = sema_module_expr_get_runtime(runtime, ctx.output);
    size_t *args = vec_new_in(module->mempool, size_t);
    vec_resize(args, vec_len(type->function.args));
    for (size_t i = 0; i < vec_len(call->args); i++) {
        SemaValueRuntime *runtime = NOT_NULL(sema_module_analyze_runtime_expr(module,
            call->args[i], sema_expr_ctx_new(ctx.output, type->function.args[i])));
        args[i] = sema_module_expr_get_runtime(runtime, ctx.output);
    }
    size_t step_id = vec_len(ctx.output->steps);
    vec_push(ctx.output->steps, ir_expr_step_new_call(args, callable));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        type->function.returns, step_id);
}


