#include "stmt.h"
#include "sema/module/module.h"

bool sema_module_stage_init_stmt(SemaModule *module, Slice slice) {
    if (sema_module_is_global_scope(module)) {
        sema_module_err(module, slice, "statements are not allowed in global scope");
        return false;
    }
    return true;
}
