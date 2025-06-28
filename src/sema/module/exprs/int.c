#include "int.h"
#include "sema/module/api/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_int(SemaModule *module, uint64_t val, SemaExprCtx ctx) {
    SemaType *type = NULL;
    if (ctx.expectation) {
        SemaType *exp = sema_type_root(ctx.expectation);
        if (exp->kind == SEMA_TYPE_INT) {
            type = ctx.expectation;
        }
    }
    if (!type) {
        type = sema_type_new_int(module->mempool, SEMA_INT_32, true);
    }
    return sema_value_new_runtime_const(module->mempool, sema_const_new_integer(module->mempool, type, val));
}


