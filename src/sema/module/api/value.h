#pragma once

#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

typedef enum {
    SEMA_RUNTIME_FINAL,
    SEMA_RUNTIME_VAR,
} SemaRuntimeKind;

typedef enum {
    SEMA_VALUE_RUNTIME_GLOBAL,
    SEMA_VALUE_RUNTIME_LOCAL,
    SEMA_VALUE_RUNTIME_EXPR_STEP,
} SemaValueRuntimeKind;

typedef struct {
    SemaRuntimeKind kind;
    SemaValueRuntimeKind val_kind;
    SemaType *type;

    union {
        IrDeclId global_id;
        IrLocalId local_id;
        struct {
            size_t step_id;
            size_t ext_of;
            bool is_ext;
        } in_expr_id;
    };
} SemaValueRuntime;

typedef struct SemaValue SemaValue;

SemaType *sema_value_should_be_type(SemaModule *module, Slice where, SemaValue *value);
SemaValueRuntime *sema_value_should_be_runtime(SemaModule *module, Slice where, SemaValue *value);
SemaType *sema_value_is_type(SemaValue *value);
SemaValueRuntime *sema_value_is_runtime(SemaValue *value);
