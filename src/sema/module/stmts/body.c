#include "body.h"
#include "ast/body.h"
#include "ast/node.h"
#include "sema/module/module.h"
#include "sema/module/stages/stages.h"

HirCode *sema_module_emit_code(SemaModule *module, AstBody *body, SemaLoop *loop) {
    sema_module_push_scope(module, loop);
    Slice unreachable;
    bool is_unreachable = false;
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        if (sema_module_scope_breaks(module)) {
            if (!is_unreachable) {
                unreachable = body->nodes[i]->slice;
                is_unreachable = true;
            } else {
                unreachable = slice_union(unreachable, body->nodes[i]->slice);
            }
        }
        for (size_t j = 0; j < sema_stages_count; j++) {
            if (!sema_stages[j](module, body->nodes[i])) {
                break;
            }
        }
    }
    if (is_unreachable) {
        sema_module_err(module, unreachable, "code is unreachable");
    }
    body->sema.breaks = sema_module_scope_breaks(module);
    if (!body->sema.breaks) {
        sema_module_emit_current_defers(module);
    }
    HirStmt *stmts = sema_ss_get_stmts(module->ss);
    sema_module_pop_scope(module);
    return hir_code_new(module->mempool, stmts);
}
