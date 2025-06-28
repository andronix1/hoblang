#include "for.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "hir/api/code.h"
#include "hir/api/expr.h"
#include "hir/api/hir.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stmts/body.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

inline SemaValueRuntime *sema_type_must_have_ext(SemaModule *module, SemaType *type, Slice where, Slice name, bool *by_ref) {
    SemaExtDecl decl;
    if (!sema_type_search_ext(module, type, name, &decl)) {
        sema_module_err(module, where, "expression type must have $S extension for using in for-loop", name);
        return NULL;
    }
    SemaValueRuntime *runtime = NOT_NULL(sema_value_should_be_runtime(module, where, decl.function));
    assert(runtime->type->kind == SEMA_TYPE_FUNCTION);
    assert(vec_len(runtime->type->function.args) > 0);
    *by_ref = decl.by_ref;
    return runtime;
}

static size_t sema_expr_output_push_maybe_ref_local(SemaExprOutput *output, HirLocalId local, bool is_ref) {
    size_t step_id = sema_expr_output_push_step(output, hir_expr_step_new_get_local(local));
    return is_ref ?
        sema_expr_output_push_step(output, hir_expr_step_new_take_ref(step_id)) :
        step_id;
}

bool sema_module_emit_stmt_for(SemaModule *module, AstFor *for_loop) {
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr_full(module, for_loop->iterator,
        sema_expr_ctx_new(&output, sema_type_new_bool(module->mempool))));
    bool get_current_by_ref, next_by_ref;
    SemaValueRuntime *get_current = NOT_NULL(sema_type_must_have_ext(module, runtime->type, for_loop->iterator->slice, 
        slice_from_cstr("getCurrent"), &get_current_by_ref));
    SemaValueRuntime *next = NOT_NULL(sema_type_must_have_ext(module, runtime->type, for_loop->iterator->slice, 
        slice_from_cstr("next"), &next_by_ref));
    SemaType *target_type = get_current->type->function.returns;

    SemaType *get_current_type_target = sema_type_new_function(module->mempool, vec_create_in(module->mempool,
        get_current->type->function.args[0]), target_type);
    if (!sema_type_eq(get_current->type, get_current_type_target)) {
        sema_module_err(module, for_loop->iterator->slice, "`getCurrent` has type $t when $t is required", next->type, get_current_type_target);
        return NULL;
    }

    SemaType *next_type_target = sema_type_new_function(module->mempool, vec_create_in(module->mempool,
        next->type->function.args[0]), sema_type_new_bool(module->mempool));
    if (!sema_type_eq(next->type, next_type_target)) {
        sema_module_err(module, for_loop->iterator->slice, "`next` has type $t when $t is required", next->type, next_type_target);
        return NULL;
    }

    /*
        var iter = ...;
        loop {
            if !next(&iter) {
                break;
            }
            var value = getCurrent(&iter);
            ...
        }
     */
    HirLocalId iter = hir_fun_add_local(module->hir, module->ss->func_id,
        hir_func_local_new(sema_type_to_hir(module, runtime->type), HIR_MUTABLE));
    sema_ss_append_stmt(module->ss, hir_stmt_new_decl_var(iter));
    sema_ss_append_stmt(module->ss, hir_stmt_new_store(hir_expr_new(vec_create_in(module->mempool,
        hir_expr_step_new_get_local(iter) 
    )), sema_expr_output_collect(&output)));

    HirLoopId id = hir_fun_add_loop(module->hir, module->ss->func_id);
    SemaLoop loop = sema_loop_new(id, for_loop->label);

    SemaExprOutput has_next = sema_expr_output_new(module->mempool);
    sema_expr_output_push_step(&has_next, hir_expr_step_new_call(
        sema_module_expr_emit_runtime(module, next, &has_next),
        vec_create_in(module->mempool, sema_expr_output_push_maybe_ref_local(&has_next, iter, next_by_ref))
    ));

    SemaExprOutput current = sema_expr_output_new(module->mempool);
    sema_expr_output_push_step(&current, hir_expr_step_new_call(
        sema_module_expr_emit_runtime(module, get_current, &current),
        vec_create_in(module->mempool, sema_expr_output_push_maybe_ref_local(&current, iter, get_current_by_ref))
    ));

    HirLocalId current_iter = hir_fun_add_local(module->hir, module->ss->func_id,
        hir_func_local_new(sema_type_to_hir(module, target_type), HIR_MUTABLE));
    HirExpr current_iter_expr = hir_expr_new(vec_create_in(module->mempool, hir_expr_step_new_get_local(current_iter)));
    HirStmt *stmts = vec_create_in(module->mempool,
        hir_stmt_new_decl_var(current_iter),
        hir_stmt_new_store(current_iter_expr, sema_expr_output_collect(&current)),
    );
    sema_module_push_scope(module, &loop);
    sema_module_push_decl(module, for_loop->iter, sema_decl_new(module->mempool, module, sema_value_new_runtime_local(
        module->mempool, SEMA_RUNTIME_VAR, target_type, current_iter)));
    HirCode *code = NOT_NULL(sema_module_emit_code(module, for_loop->body, NULL));
    sema_module_pop_scope(module);
    vec_extend(stmts, code->stmts);
    code = hir_code_new(module->mempool, stmts);

    HirCode *break_loop = hir_code_new(module->mempool, vec_create_in(module->mempool, hir_stmt_new_break(id)));

    HirStmt exec_if_next = hir_stmt_new_cond_jmp(vec_create_in(module->mempool, 
        hir_stmt_cond_jmp_block(sema_expr_output_collect(&has_next), code),
    ), break_loop);

    sema_ss_append_stmt(module->ss, hir_stmt_new_loop(id, hir_code_new(module->mempool, vec_create_in(module->mempool,
        exec_if_next
    ))));
    return true;
}
