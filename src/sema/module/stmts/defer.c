#include "defer.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/stmts/body.h"

bool sema_module_emit_stmt_defer(SemaModule *module, AstBody *defer) {
    sema_module_add_defer(module, NOT_NULL(sema_module_emit_code(module, defer, NULL)));
    return true;
}
