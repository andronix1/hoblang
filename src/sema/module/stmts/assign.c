#include "assign.h"
#include "core/mempool.h"
#include "core/null.h"
#include "hir/api/hir.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/exprs/binop.h"
#include "sema/module/type.h"
#include <assert.h>
#include <stdio.h>

bool sema_module_emit_stmt_assign(SemaModule *module, AstAssign *assign) {
    SemaExprOutput dsto = sema_expr_output_new(module->mempool);
    SemaValueRuntime *lvalue = NOT_NULL(sema_module_emit_runtime_expr_full(module,
        assign->dst, sema_expr_ctx_new(&dsto, NULL)));

    if (lvalue->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, assign->dst->slice, "this expression is not assignable");
    }

    SemaExprOutput valueo = sema_expr_output_new(module->mempool);
    SemaValueRuntime *rvalue = NOT_NULL(sema_module_emit_runtime_expr_full(module,
        assign->what, sema_expr_ctx_new(&valueo, lvalue->type)));
    if (lvalue->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, assign->dst->slice, "this expression is not assignable");
        return false;
    }
    if (!sema_type_can_be_downcasted(lvalue->type, rvalue->type)) {
        sema_module_err(module, assign->what->slice, "cannot assign expression of type $t to assignable expression of type $t", rvalue->type, lvalue->type);
        return false;
    }
    HirExpr value_expr = sema_expr_output_collect(&valueo);
    if (assign->short_assign.is) {
        HirTypeId type = sema_type_hir_id(lvalue->type);
        
        HirLocalId temp = hir_fun_add_local(module->hir, module->ss->func_id, hir_func_local_new(
            sema_type_hir_id(sema_type_new_pointer(module, lvalue->type)), HIR_IMMUTABLE));

        sema_expr_output_push_step(&dsto, hir_expr_step_new_take_ref(sema_expr_output_last_id(&dsto)));
        sema_ss_append_stmt(module->ss, hir_stmt_new_init_final(temp, sema_expr_output_collect(&dsto)));

        HirLocalId value = hir_fun_add_local(module->hir, module->ss->func_id, hir_func_local_new(type, HIR_IMMUTABLE));
        sema_ss_append_stmt(module->ss, hir_stmt_new_init_final(value, value_expr));

        SemaExprOutput output = sema_expr_output_new_with(vec_create_in(module->mempool,
            hir_expr_step_new_get_local(temp),
            hir_expr_step_new_deref(0),
            hir_expr_step_new_get_local(value)
        ));
        NOT_NULL(sema_module_append_expr_binop(module, lvalue->type, 1, 2, &assign->short_assign.kind, &output));
        
        sema_ss_append_stmt(module->ss, hir_stmt_new_store(hir_expr_new(vec_create_in(module->mempool, 
            hir_expr_step_new_get_local(temp),
            hir_expr_step_new_deref(0),
        )), sema_expr_output_collect(&output)));
        return true;
    }
    sema_ss_append_stmt(module->ss, hir_stmt_new_store(sema_expr_output_collect(&dsto), value_expr));
    return true;
}
