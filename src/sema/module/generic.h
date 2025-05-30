#pragma once

#include "sema/module/generics/type.h"
#include "sema/module/type.h"

typedef enum {
    SEMA_GENERIC_TYPE
} SemaGenericKind;

typedef struct SemaGeneric {
    SemaGenericKind kind;
    SemaGenericParam *params;
    SemaModule *in;

    union {
        SemaGenericTypeInfo type;
    };
} SemaGeneric;

SemaGeneric *sema_generic_new(SemaModule *module, SemaType *type, SemaGenericParam *params);
