#pragma once

#include "ir/api/type.h"
#include <stddef.h>

typedef struct SemaTypeAlias SemaTypeAlias;
typedef size_t SemaTypeId;
typedef struct SemaType SemaType;

bool sema_type_eq(const SemaType *a, const SemaType *b);
IrTypeId sema_type_ir_id(SemaType *type);
