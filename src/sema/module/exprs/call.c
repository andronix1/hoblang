#include "call.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include <stdio.h>

SemaValue *sema_module_emit_expr_call(SemaModule *module, AstCall *call, SemaExprCtx ctx) {
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module,
        call->inner, sema_expr_ctx_new(ctx.output, NULL))); 
    bool is_ext = false;
    size_t ext_step_id = 0;
    if (runtime->val_kind == SEMA_VALUE_RUNTIME_EXPR_STEP && runtime->in_expr_id.is_ext) {
        is_ext = true;
        ext_step_id = runtime->in_expr_id.ext_of;
    }
    SemaType *type = sema_type_resolve(runtime->type);
    if (type->kind != SEMA_TYPE_FUNCTION) {
        sema_module_err(module, call->inner->slice, "$t is not callable", type);
        return NULL;
    }
    if (vec_len(type->function.args) != vec_len(call->args) + is_ext) {
        sema_module_err(module, call->inner->slice, "expected to $l arguments to be passed but $l was passed", vec_len(type->function.args) - is_ext, vec_len(call->args));
        return NULL;
    }
    size_t callable = sema_module_expr_emit_runtime(runtime, ctx.output);
    size_t *args = vec_new_in(module->mempool, size_t);
    vec_reserve(args, vec_len(type->function.args) + is_ext);
    if (is_ext) {
        vec_push(args, ext_step_id);
    }
    for (size_t i = 0; i < vec_len(call->args); i++) {
        SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module,
            call->args[i], sema_expr_ctx_new(ctx.output, type->function.args[i + is_ext])));
        if (!sema_type_eq(runtime->type, type->function.args[i + is_ext])) {
            sema_module_err(module, call->args[i]->slice, "expected $t, but $t was passed",
                type->function.args[i + is_ext], runtime->type);
        }
        vec_push(args, sema_module_expr_emit_runtime(runtime, ctx.output));
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_call(callable, args));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        type->function.returns, step_id);
}


