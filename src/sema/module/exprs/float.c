#include "float.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_float(SemaModule *module, long double val, SemaExprCtx ctx) {
    SemaType *type = NULL;
    if (ctx.expectation) {
        ctx.expectation = sema_type_root(ctx.expectation);
        if (ctx.expectation->kind == SEMA_TYPE_FLOAT) {
            type = ctx.expectation;
        }
    }
    if (!type) {
        type = sema_type_new_float(module->mempool, SEMA_FLOAT_32);
    }
    return sema_value_new_runtime_const(module->mempool, sema_const_new_float(module->mempool, type, val));
}


