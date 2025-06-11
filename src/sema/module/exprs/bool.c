#include "bool.h"
#include "core/vec.h"
#include "ir/stmt/expr.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_bool(SemaModule *module, bool val, SemaExprCtx ctx) {
    size_t step_id = vec_len(ctx.output->steps);
    vec_push(ctx.output->steps, ir_expr_step_new_bool(val));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        sema_type_new_bool(module), step_id);
}
