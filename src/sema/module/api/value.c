#include "value.h"
#include "sema/module/value.h"

SemaType *sema_value_is_runtime(SemaValue *value) {
    if (value->kind != SEMA_VALUE_RUNTIME) {
        return NULL;
    }
    return value->runtime.type;
}

SemaType *sema_value_is_type(SemaValue *value) {
    if (value->kind != SEMA_VALUE_TYPE) {
        return NULL;
    }
    return value->type;
}
