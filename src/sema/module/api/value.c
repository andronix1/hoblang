#include "value.h"
#include "core/null.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

inline SemaGeneric *sema_value_is_generic(SemaValue *value) {
    if (value->kind != SEMA_VALUE_GENERIC) {
        return NULL;
    }
    return value->generic;
}

inline SemaConst *sema_value_runtime_is_const(SemaValueRuntime *runtime) {
    if (runtime->val_kind == SEMA_VALUE_RUNTIME_CONST) {
        return runtime->constant;
    }
    return NULL;
}

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

SemaConst *sema_value_runtime_should_be_constant(SemaModule *module, Slice where, SemaValueRuntime *runtime) {
    if (runtime->val_kind != SEMA_VALUE_RUNTIME_CONST) {
        sema_module_err(module, where, "value is not a constant");
        return NULL;
    }
    return runtime->constant;
}

SemaGeneric *sema_value_should_be_generic(SemaModule *module, Slice where, SemaValue *value) {
    if (value->kind != SEMA_VALUE_GENERIC) {
        sema_module_err(module, where, "value is not a generic");
        return NULL;
    }
    return value->generic;
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
