#pragma once

#include "hir/api/type.h"
#include "hir/api/type.h"
#include "sema/module/api/module.h"
#include <stddef.h>
#include <stdarg.h>

typedef struct SemaTypeAlias SemaTypeAlias;
typedef size_t SemaTypeId;
typedef struct SemaType SemaType;

SemaType *sema_type_replace(Mempool *mempool, SemaType *source, SemaType **from, SemaType **to);

bool sema_type_can_be_downcasted(SemaType *type, SemaType *to);
bool sema_type_can_be_casted(SemaType *type, SemaType *to);

SemaType *sema_type_root(SemaType *type);
HirType *sema_type_to_hir(SemaModule* module, SemaType *type);

void sema_type_print(va_list list);
