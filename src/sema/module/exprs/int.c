#include "int.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_int(SemaModule *module, uint64_t val, SemaExprCtx ctx) {
    SemaType *type = NULL;
    if (ctx.expectation) {
        ctx.expectation = sema_type_resolve(ctx.expectation);
        if (ctx.expectation->kind == SEMA_TYPE_INT) {
            type = ctx.expectation;
        }
    }
    if (!type) {
        type = sema_type_new_int(module, SEMA_INT_32, true);
    }
    return sema_value_new_runtime_const(module->mempool, sema_const_new_integer(module->mempool, type, val));
}


