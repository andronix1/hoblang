#include "value.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

inline SemaType *sema_value_is_type(SemaValue *value) {
    if (value->kind != SEMA_VALUE_TYPE) {
        return NULL;
    }
    return value->type;
}

inline SemaValueRuntime *sema_value_is_runtime(SemaValue *value) {
    if (value->kind != SEMA_VALUE_RUNTIME) {
        return NULL;
    }
    return &value->runtime;
}

inline SemaModule *sema_value_is_module(SemaValue *value) {
    if (value->kind != SEMA_VALUE_MODULE) {
        return NULL;
    }
    return value->module;
}

SemaType *sema_value_should_be_type(SemaModule *module, Slice where, SemaValue *value) {
    SemaType *type = sema_value_is_type(value);
    if (!type) {
        sema_module_err(module, where, "expected type got $v", value);
    }
    return type;
}

SemaValueRuntime *sema_value_should_be_runtime(SemaModule *module, Slice where, SemaValue *value) {
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (!runtime) {
        sema_module_err(module, where, "expected value got $v", value);
    }
    return runtime;
}
