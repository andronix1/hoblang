#include "while.h"
#include "core/mempool.h"
#include "core/null.h"
#include "ir/api/ir.h"
#include "ir/api/stmt/code.h"
#include "ir/stmt/code.h"
#include "ir/stmt/expr.h"
#include "ir/stmt/stmt.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"
#include "sema/module/stmts/body.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"

bool sema_module_emit_stmt_while(SemaModule *module, AstWhile *while_loop) {
    SemaExprOutput output = sema_expr_output_new(module->mempool);
    SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, while_loop->cond,
        sema_expr_ctx_new(&output, sema_type_new_bool(module))));
    if (runtime->type->kind != SEMA_TYPE_BOOL) {
        sema_module_err(module, while_loop->cond->slice, "only booleans can be used in while statement conditions, but $t passed", runtime->type);
        return false;
    }

    IrLoopId id = ir_func_add_loop(module->ir, module->ss->func_id);
    sema_module_push_loop(module, while_loop->label.has ?
            sema_loop_new_labeled(id, while_loop->label.name) :
            sema_loop_new(id));
    IrCode *code = NOT_NULL(sema_module_emit_code(module, while_loop->body));
    sema_module_pop_loop(module);

    IrCode *break_loop = ir_code_new(module->mempool, vec_create_in(module->mempool, 
        ir_stmt_new_break(module->mempool, id)
    ));

    if (while_loop->is_do_while) {
        sema_expr_output_push_step(&output, ir_expr_step_new_not(sema_expr_output_last_id(&output)));
        vec_push(code->stmts, 
            ir_stmt_new_cond_jmp(module->mempool, vec_create_in(module->mempool,
                ir_stmt_cond_jmp_block(sema_expr_output_collect(&output), break_loop),
            ), NULL)
        );
    } else {
        IrStmtCondJmpBlock *conds = vec_create_in(module->mempool,
            ir_stmt_cond_jmp_block(sema_expr_output_collect(&output), code),
        );
        code = ir_code_new(module->mempool, vec_create_in(module->mempool, 
            ir_stmt_new_cond_jmp(module->mempool, conds, break_loop)
        ));
    }
    sema_ss_append_stmt(module->ss, ir_stmt_new_loop(module->mempool, id, code));
    return true;
}
