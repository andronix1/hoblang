#pragma once

#include "core/mempool.h"
#include "ir/api/decl.h"
#include "ir/api/local.h"
#include "sema/module/api/type.h"
#include <stdarg.h>

typedef enum {
    SEMA_VALUE_TYPE
} SemaValueKind;

typedef enum {
    SEMA_VALUE_ID_GLOBAL,
    SEMA_VALUE_ID_LOCAL,
    SEMA_VALUE_ID_EXPR_STEP,
} SemaValueIdKind;

typedef struct {
    union {
        IrDeclId global_id;
        IrLocalId local_id;
        size_t step_id;
    };
} SemaValueId;

typedef struct SemaValue {
    SemaValueKind kind;

    union {
        SemaType *type;
    };
} SemaValue;

SemaValue *sema_value_new_type(Mempool *mempool, SemaType *type);
void sema_value_print(va_list list);
