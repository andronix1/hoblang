#include "return.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/exprs/expr.h"

bool sema_module_emit_stmt_return(SemaModule *module, AstReturn *ret) {
    if (!ret->value) {
        TODO;
        sema_ss_append_stmt(module->ss, hir_stmt_new_ret_void());
        return true;
    }

    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, ret->value,
        sema_expr_ctx_new(&output, module->ss->returns)));

    if (!sema_type_eq(module->ss->returns, runtime->type)) {
        sema_module_err(module, ret->value->slice, "expected to return $t, got $t", module->ss->returns, runtime->type);
        return false;
    }
    sema_module_emit_defers(module);
    sema_ss_append_stmt(module->ss, hir_stmt_new_ret(hir_expr_new(output.steps)));
    sema_module_scope_break(module);
    return true;
}
