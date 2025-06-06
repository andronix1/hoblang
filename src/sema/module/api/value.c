#include "value.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

inline SemaType *sema_value_is_type(SemaValue *value) {
    if (value->kind != SEMA_VALUE_TYPE) {
        return NULL;
    }
    return value->type;
}

SemaType *sema_value_should_be_type(SemaModule *module, Slice where, SemaValue *value) {
    SemaType *type = sema_value_is_type(value);
    if (!type) {
        sema_module_err(module, where, "expected type got $v", value);
    }
    return type;
}
