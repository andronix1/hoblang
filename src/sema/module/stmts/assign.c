#include "assign.h"
#include "core/null.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stmts/expr.h"
#include <assert.h>

bool sema_module_emit_assign(SemaModule *module, AstAssign *assign) {
    assert(!assign->short_assign.is);
    SemaExprOutput dsto = sema_expr_output_new(module->mempool);
    SemaValueRuntime *lvalue = NOT_NULL(sema_module_analyze_runtime_expr(module,
        assign->dst, sema_expr_ctx_new(&dsto, NULL)));
    SemaExprOutput valueo = sema_expr_output_new(module->mempool);
    SemaValueRuntime *rvalue = NOT_NULL(sema_module_analyze_runtime_expr(module,
        assign->what, sema_expr_ctx_new(&valueo, NULL)));
    if (lvalue->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, assign->dst->slice, "this expression is not assignable");
        return false;
    }
    if (!sema_type_eq(lvalue->type, rvalue->type)) {
        sema_module_err(module, assign->what->slice, "cannot assign expression of type $t to assignable expression of type $t", rvalue->type, lvalue->type);
        return false;
    }
    sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool,
        ir_expr_new(dsto.steps),
        ir_expr_new(valueo.steps)
    ));
    return true;
}
