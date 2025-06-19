#include "idx.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_idx(SemaModule *module, AstExprIdx *idx, SemaExprCtx ctx) {
    SemaValueRuntime *inner_runtime = NOT_NULL(sema_module_emit_runtime_expr(module, idx->inner,
        sema_expr_ctx_new(ctx.output, NULL)));
    SemaValueRuntime *idx_runtime = NOT_NULL(sema_module_emit_runtime_expr(module, idx->idx,
        sema_expr_ctx_new(ctx.output, NULL)));
    size_t value_step = sema_module_expr_emit_runtime(inner_runtime, ctx.output);
    size_t idx_step = sema_module_expr_emit_runtime(idx_runtime, ctx.output);
    SemaType *type = inner_runtime->type;
    if (type->kind == SEMA_TYPE_ARRAY) {
        size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_idx_array(value_step, idx_step));
        return sema_value_new_runtime_expr_step(module->mempool, inner_runtime->kind, type->array.of, step_id);
    }
    if (type->kind == SEMA_TYPE_POINTER) {
        size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_idx_pointer(value_step, idx_step));
        return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_VAR, type->pointer_to, step_id);
    }
    sema_module_err(module, idx->inner->slice, "only array or pointers can be indexed, not $t", type);
    return NULL;
}


