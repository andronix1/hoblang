#include "char.h"
#include "core/vec.h"
#include "ir/stmt/expr.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_analyze_expr_char(SemaModule *module, char val, SemaExprCtx ctx) {
    SemaType *u8 = sema_type_new_int(module, SEMA_INT_8, false);
    size_t step_id = vec_len(ctx.output->steps);
    vec_push(ctx.output->steps, ir_expr_step_new_int(sema_type_ir_id(u8), val));
    return sema_value_new_runtime_expr_step(module->mempool, u8, step_id);
}


