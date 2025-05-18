#include "while.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/nodes/body.h"
#include "sema/module/nodes/expr.h"
#include "sema/module/type.h"

bool sema_module_analyze_while(SemaModule *module, AstWhile *while_loop) {
    SemaType *boolean = sema_type_new_primitive_bool(module->mempool);
    SemaType *type = sema_value_is_runtime(NOT_NULL(sema_module_analyze_expr(module, while_loop->cond,
        sema_expr_ctx_new(boolean))));
    if (!type) {
        sema_module_err(module, while_loop->cond->slice, "condition cannot be non-runtime value", type);
    } else if (!sema_type_eq(type, boolean)) {
        sema_module_err(module, while_loop->cond->slice, "expected boolean got $t", type);
    }
    sema_module_analyze_body(module, while_loop->body);
    return false;
}
