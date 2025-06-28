#include "char.h"
#include "hir/api/expr.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_char(SemaModule *module, char val, SemaExprCtx ctx) {
    SemaType *u8 = sema_type_new_int(module->mempool, SEMA_INT_8, false);
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_const(
        hir_const_new_int(sema_type_to_hir(module, u8), val)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, u8, step_id);
}


