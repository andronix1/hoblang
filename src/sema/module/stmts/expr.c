#include "expr.h"
#include "core/null.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"

bool sema_module_emit_stmt_expr(SemaModule *module, AstExpr *expr) {
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValue *value = NOT_NULL(sema_module_emit_expr(module, expr, sema_expr_ctx_new(&output, NULL)));
    sema_ss_append_stmt(module->ss, hir_stmt_new_expr(sema_expr_output_collect(&output)));
    if (sema_value_is_finish(value)) {
        sema_module_scope_break(module);
        sema_ss_append_stmt(module->ss, hir_stmt_new_unreachable());
    }
    return true;
}


