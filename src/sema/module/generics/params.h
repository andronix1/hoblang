#pragma once

#include "core/slice.h"
#include "sema/module/api/type.h"

typedef struct {
    Slice name;
    SemaType *type;
} SemaGenericParam;

SemaGenericParam sema_generic_param_new(Slice name, SemaType *type);
void sema_generic_param_add_variant(SemaGenericParam *param, SemaType *type);
