#include "value.h"
#include "sema/module/value.h"

SemaType *sema_value_is_var(SemaValue *value) {
    if (value->kind != SEMA_VALUE_RUNTIME || value->runtime.kind != SEMA_VALUE_RUNTIME_VAR) {
        return NULL;
    }
    return value->runtime.type;
}

SemaGeneric *sema_value_is_generic(SemaValue *value) {
    if (value->kind != SEMA_VALUE_GENERIC) {
        return NULL;
    }
    return value->generic;
}

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
