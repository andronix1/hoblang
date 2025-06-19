#include "while.h"
#include "core/mempool.h"
#include "core/null.h"
#include "hir/api/hir.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stmts/body.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"

bool sema_module_emit_stmt_while(SemaModule *module, AstWhile *while_loop) {
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr_full(module, while_loop->cond,
        sema_expr_ctx_new(&output, sema_type_new_bool(module))));
    if (runtime->type->kind != SEMA_TYPE_BOOL) {
        sema_module_err(module, while_loop->cond->slice, "only booleans can be used in while statement conditions, but $t passed", runtime->type);
        return false;
    }

    HirLoopId id = hir_fun_add_loop(module->hir, module->ss->func_id);
    SemaLoop loop = sema_loop_new(id, while_loop->label);
    HirCode *code = NOT_NULL(sema_module_emit_code(module, while_loop->body, &loop));

    HirCode *break_loop = hir_code_new(module->mempool, vec_create_in(module->mempool, hir_stmt_new_break(id)));

    if (while_loop->is_do_while) {
        sema_expr_output_push_step(&output, hir_expr_step_new_not(sema_expr_output_last_id(&output)));
        vec_push(code->stmts, 
            hir_stmt_new_cond_jmp(vec_create_in(module->mempool,
                hir_stmt_cond_jmp_block(sema_expr_output_collect(&output), break_loop),
            ), NULL)
        );
    } else {
        HirStmtCondJmpBlock *conds = vec_create_in(module->mempool,
            hir_stmt_cond_jmp_block(sema_expr_output_collect(&output), code),
        );
        code = hir_code_new(module->mempool, vec_create_in(module->mempool, 
            hir_stmt_new_cond_jmp(conds, break_loop)
        ));
    }
    sema_ss_append_stmt(module->ss, hir_stmt_new_loop(id, code));
    return true;
}
