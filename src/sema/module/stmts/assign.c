#include "assign.h"
#include "core/mempool.h"
#include "core/null.h"
#include "ir/api/ir.h"
#include "ir/func.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/type.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/exprs/binop.h"
#include "sema/module/type.h"
#include <assert.h>

bool sema_module_emit_stmt_assign(SemaModule *module, AstAssign *assign) {
    SemaExprOutput dsto = sema_expr_output_new(module->mempool);
    SemaValueRuntime *lvalue = NOT_NULL(sema_module_emit_runtime_expr(module,
        assign->dst, sema_expr_ctx_new(&dsto, NULL)));

    if (lvalue->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, assign->dst->slice, "this expression is not assignable");
    }

    SemaExprOutput valueo = sema_expr_output_new(module->mempool);
    SemaValueRuntime *rvalue = NOT_NULL(sema_module_emit_runtime_expr(module,
        assign->what, sema_expr_ctx_new(&valueo, lvalue->type)));
    if (lvalue->kind != SEMA_RUNTIME_VAR) {
        sema_module_err(module, assign->dst->slice, "this expression is not assignable");
        return false;
    }
    if (!sema_type_eq(lvalue->type, rvalue->type)) {
        sema_module_err(module, assign->what->slice, "cannot assign expression of type $t to assignable expression of type $t", rvalue->type, lvalue->type);
        return false;
    }
    IrExprStep *steps = valueo.steps;
    if (assign->short_assign.is) {
        IrTypeId type = sema_type_ir_id(lvalue->type);
        
        IrLocalId temp = ir_func_add_local(module->ir, module->ss->func_id, ir_func_local_new(IR_IMMUTABLE, 
            sema_type_ir_id(sema_type_new_pointer(module, lvalue->type))));

        vec_push(dsto.steps, ir_expr_step_new_take_ref(vec_len(dsto.steps) - 1));
        sema_ss_append_stmt(module->ss, ir_stmt_new_init_final(module->mempool, temp, ir_expr_new(dsto.steps)));

        IrLocalId value = ir_func_add_local(module->ir, module->ss->func_id, ir_func_local_new(IR_IMMUTABLE, type));
        sema_ss_append_stmt(module->ss, ir_stmt_new_init_final(module->mempool, value, ir_expr_new(steps)));

        SemaExprOutput output = sema_expr_output_new_with(vec_create_in(module->mempool,
            ir_expr_step_new_get_local(temp),
            ir_expr_step_new_deref(0),
            ir_expr_step_new_get_local(value)
        ));
        NOT_NULL(sema_module_append_expr_binop(module, lvalue->type, 1, 2, &assign->short_assign.kind, &output));
        
        sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool, ir_expr_new(vec_create_in(module->mempool, 
            ir_expr_step_new_get_local(temp),
            ir_expr_step_new_deref(0),
        )), sema_expr_output_collect(&output)));
        return true;
    }
    sema_ss_append_stmt(module->ss, ir_stmt_new_store(module->mempool, sema_expr_output_collect(&dsto), ir_expr_new(steps)));
    return true;
}
