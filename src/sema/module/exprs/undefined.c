#include "undefined.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_undefined(SemaModule *module, Slice where, SemaExprCtx ctx) {
    if (!ctx.expectation) {
        sema_module_err(module, where, "undefined expression can be used when there is any type expects");
        return NULL;
    }
    return sema_value_new_runtime_const(module->mempool, sema_const_new_undefined(module->mempool, ctx.expectation));
}


