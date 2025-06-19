#include "bool.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_bool(SemaModule *module, bool val, SemaExprCtx ctx) {
    SemaType *boolean = sema_type_new_bool(module);
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_const(
        hir_const_new_bool(sema_type_hir_id(boolean), val)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, boolean, step_id);
}
