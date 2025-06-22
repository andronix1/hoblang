#include "string.h"
#include "core/null.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"
#include "sema/module/std.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_string(SemaModule *module, Slice string_slice, Slice string, SemaExprCtx ctx) {
    SemaType *type = NOT_NULL(sema_module_std_string(module, string_slice));
    SemaValueRuntime *new_string = NOT_NULL(sema_module_std_string_new(module, string_slice));
    size_t str = sema_expr_output_push_step(ctx.output, hir_expr_step_new_string(string));
    size_t length = sema_expr_output_push_step(ctx.output, hir_expr_step_new_const(
        hir_const_new_int(sema_type_hir_id(sema_module_std_usize(module, string_slice)), string.length)));
    size_t callable = sema_module_expr_emit_runtime(module, new_string, ctx.output);
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_call(callable,
        vec_create_in(module->mempool, str, length)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
}


