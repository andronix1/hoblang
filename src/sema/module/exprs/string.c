#include "string.h"
#include "core/null.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/std.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_string(SemaModule *module, Slice string_slice, Slice string) {
    SemaType *type = NOT_NULL(sema_module_std_string(module, string_slice));
    return sema_value_new_runtime_const(module->mempool, sema_const_new_string(module->mempool, type, string));
}


