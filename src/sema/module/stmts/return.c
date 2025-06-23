#include "return.h"
#include "core/mempool.h"
#include "core/null.h"
#include "hir/api/expr.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"

bool sema_module_emit_stmt_return(SemaModule *module, AstReturn *ret) {
    if (!ret->value) {
        if (!sema_type_can_be_downcasted(module->ss->returns, sema_type_new_void(module))) {
            sema_module_err(module, ret->slice, "expected function to return $t", module->ss->returns);
        }
        sema_ss_append_stmt(module->ss, hir_stmt_new_ret_void());
        return true;
    }

    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr_full(module, ret->value,
        sema_expr_ctx_new(&output, module->ss->returns)));

    if (!sema_type_can_be_downcasted(module->ss->returns, runtime->type)) {
        sema_module_err(module, ret->value->slice, "expected to return $t, got $t", module->ss->returns, runtime->type);
        return false;
    }
    HirLocalId local_id = hir_fun_add_local(module->hir, module->ss->func_id, hir_func_local_new(
        sema_type_hir_id(runtime->type), HIR_IMMUTABLE));
    sema_ss_append_stmt(module->ss, hir_stmt_new_init_final(local_id, hir_expr_new(output.steps)));
    sema_module_emit_defers(module);
    sema_ss_append_stmt(module->ss, hir_stmt_new_ret(hir_expr_new(vec_create_in(module->mempool,
        hir_expr_step_new_get_local(local_id)
    ))));
    sema_module_scope_break(module);
    return true;
}
