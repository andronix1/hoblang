#pragma once

#include "sema/module/api.h"
#include "sema/module/api/type.h"
#include "sema/module/generics/params.h"

typedef struct SemaGenericTypeUsage {
    SemaType **params;
    SemaType *type;
} SemaGenericTypeUsage;

typedef struct {
    SemaType *type;
    SemaGenericTypeUsage *usages;
    SemaGenericParam *params;
    bool locked;
} SemaGenericTypeInfo;

SemaType *sema_type_generate(SemaModule *module, Slice where, SemaGenericTypeInfo *info, SemaGenericParam *params, SemaType **iparams);
SemaGenericTypeInfo sema_generic_type_info_new(Mempool *mempool, SemaGenericParam *params, SemaType *type);
bool sema_generic_type_expand_usages(Mempool *mempool, SemaGenericTypeInfo *info);
