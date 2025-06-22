#pragma once

#include "sema/module/api/const.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"
#include "sema/module/api/generic.h"

typedef enum {
    SEMA_RUNTIME_FINAL,
    SEMA_RUNTIME_VAR,
} SemaRuntimeKind;

typedef enum {
    SEMA_VALUE_RUNTIME_GLOBAL,
    SEMA_VALUE_RUNTIME_LOCAL,
    SEMA_VALUE_RUNTIME_CONST,
    SEMA_VALUE_RUNTIME_EXPR_STEP,
} SemaValueRuntimeKind;

typedef struct {
    SemaRuntimeKind kind;
    SemaValueRuntimeKind val_kind;
    SemaType *type;

    union {
        HirDeclId global_id;
        HirLocalId local_id;
        SemaConst *constant;
        struct {
            size_t step_id;
            size_t ext_of;
            bool is_ext;
        } in_expr_id;
    };
} SemaValueRuntime;

typedef struct SemaValue SemaValue;

SemaType *sema_value_should_be_type(SemaModule *module, Slice where, SemaValue *value);
SemaConst *sema_value_runtime_should_be_constant(SemaModule *module, Slice where, SemaValueRuntime *runtime);
SemaValueRuntime *sema_value_should_be_runtime(SemaModule *module, Slice where, SemaValue *value);
SemaGeneric *sema_value_should_be_generic(SemaModule *module, Slice where, SemaValue *value);
SemaGeneric *sema_value_is_generic(SemaValue *value);
SemaType *sema_value_is_type(SemaValue *value);
SemaModule *sema_value_is_module(SemaValue *value);
SemaValueRuntime *sema_value_is_runtime(SemaValue *value);
