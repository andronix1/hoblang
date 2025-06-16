#include "loop_control.h"
#include "core/null.h"
#include "ir/stmt/stmt.h"
#include "sema/module/module.h"
#include "sema/module/scope.h"

static inline IrLoopId *sema_resolve_loop(SemaModule *module, AstLoopControl *control) {
    return control->is_labeled ?
        sema_ss_labeled_loop(module->ss, control->label) :
        sema_ss_top_loop(module->ss);
}

bool sema_module_emit_stmt_break(SemaModule *module, AstLoopControl *control) {
    IrLoopId id = *NOT_NULL(sema_resolve_loop(module, control));
    sema_module_emit_defers_before_loop(module, id);
    sema_ss_append_stmt(module->ss, ir_stmt_new_break(module->mempool, id));
    sema_module_scope_break(module);
    return true;
}

bool sema_module_emit_stmt_continue(SemaModule *module, AstLoopControl *control) {
    IrLoopId id = *NOT_NULL(sema_resolve_loop(module, control));
    sema_module_emit_defers_before_loop(module, id);
    sema_ss_append_stmt(module->ss, ir_stmt_new_continue(module->mempool, id));
    sema_module_scope_break(module);
    return true;
}
