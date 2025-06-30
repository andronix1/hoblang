#include "bool.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_bool(SemaModule *module, bool val) {
    return sema_value_new_runtime_const(module->mempool, sema_const_new_bool(module->mempool, val));
}
