#include "int.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "ir/stmt/expr.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_int(SemaModule *module, uint64_t val, SemaExprCtx ctx) {
    SemaType *type = NULL;
    if (ctx.expectation) {
        if (ctx.expectation->kind == SEMA_TYPE_INT) {
            type = ctx.expectation;
        }
    }
    if (!type) {
        type = sema_type_new_int(module, SEMA_INT_32, true);
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, ir_expr_step_new_int(sema_type_ir_id(type), val));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
}


