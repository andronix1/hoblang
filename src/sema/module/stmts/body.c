#include "body.h"
#include "ast/body.h"
#include "ir/stmt/code.h"
#include "sema/module/module.h"
#include "sema/module/stages/stages.h"

IrCode *sema_module_emit_code(SemaModule *module, AstBody *body) {
    sema_module_push_scope(module);
    for (size_t i = 0; i < vec_len(body->nodes); i++) {
        for (size_t j = 0; j < sema_stages_count; j++) {
            if (!sema_stages[j](module, body->nodes[i])) {
                break;
            }
        }
    }
    IrStmt **stmts = sema_ss_get_stmts(module->ss);
    sema_module_pop_scope(module);
    return ir_code_new(module->mempool, stmts);
}
