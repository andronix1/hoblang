#include "as.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_as(SemaModule *module, AstAs *as, SemaExprCtx ctx) {
    SemaType *real_dest = NOT_NULL(sema_module_type(module, as->type));
    SemaType *dest = sema_type_resolve(real_dest);
    SemaValueRuntime *source_runtime = NOT_NULL(sema_module_emit_runtime_expr(module, as->inner,
        sema_expr_ctx_new(ctx.output, NULL)));
    SemaType *source = source_runtime->type;

    if (sema_type_eq(source, dest)) {
        size_t step_id = sema_module_expr_emit_runtime(source_runtime, ctx.output);
        return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, real_dest, step_id);
    }

    if (source->kind == SEMA_TYPE_INT && dest->kind == SEMA_TYPE_INT) {
        size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_cast_int(
            sema_module_expr_emit_runtime(source_runtime, ctx.output), sema_type_hir_id(source),
            sema_type_hir_id(dest)));
        return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, real_dest, step_id);
    } else {
        sema_module_err(module, as->slice, "$t cannot be casted to $t", source, real_dest);
        return NULL;
    }

}
