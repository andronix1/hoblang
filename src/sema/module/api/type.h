#pragma once

#include "ir/api/type.h"
#include <stddef.h>
#include <stdarg.h>

typedef struct SemaTypeAlias SemaTypeAlias;
typedef size_t SemaTypeId;
typedef struct SemaType SemaType;

bool sema_type_eq(const SemaType *a, const SemaType *b);
IrTypeId sema_type_ir_id(SemaType *type);

void sema_type_print(va_list list);
