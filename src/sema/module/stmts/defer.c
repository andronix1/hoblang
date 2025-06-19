#include "defer.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"

static HirCode *sema_module_defer_body(SemaModule *module, AstDefer *defer) {
    switch (defer->kind) {
        case AST_DEFER_BODY: return sema_module_emit_code(module, defer->body, NULL);
        case AST_DEFER_EXPR: {
            SemaExprOutput output = sema_expr_output_new(module->mempool);
            sema_module_emit_runtime_expr_full(module, defer->expr, sema_expr_ctx_new(&output, NULL));
            return hir_code_new(module->mempool, vec_create_in(module->mempool, 
                hir_stmt_new_expr(sema_expr_output_collect(&output))));
        }
    }
    UNREACHABLE;
}

bool sema_module_emit_stmt_defer(SemaModule *module, AstDefer *defer) {
    sema_module_add_defer(module, NOT_NULL(sema_module_defer_body(module, defer)));
    return true;
}
