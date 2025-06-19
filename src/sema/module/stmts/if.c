#include "if.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"

bool sema_module_emit_stmt_if(SemaModule *module, AstIf *if_else) {
    HirStmtCondJmpBlock *conds = vec_new_in(module->mempool, HirStmtCondJmpBlock);
    vec_resize(conds, vec_len(if_else->conds));
    for (size_t i = 0; i < vec_len(if_else->conds); i++) {
        AstCondBlock *block = &if_else->conds[i];
        SemaExprOutput output = sema_expr_output_new(module->mempool);
        SemaType *boolean = sema_type_new_bool(module);
        SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr_full(module,
            block->cond, sema_expr_ctx_new(&output, boolean)));
        if (!sema_type_eq(boolean, runtime->type)) {
            sema_module_err(module, block->cond->slice, "only booleans can be used in if statement conditions, but $t passed", runtime->type);
        }
        HirCode *body = NOT_NULL(sema_module_emit_code(module, block->body, NULL));
        conds[i] = hir_stmt_cond_jmp_block(sema_expr_output_collect(&output), body);
    }
    HirCode *else_body = NULL;
    if (if_else->else_body) {
        else_body = NOT_NULL(sema_module_emit_code(module, if_else->else_body, NULL));
    }
    sema_ss_append_stmt(module->ss, hir_stmt_new_cond_jmp(conds, else_body));
    return true;
}
