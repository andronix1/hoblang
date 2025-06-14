#pragma once

#include "core/slice.h"
#include "ir/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include <stddef.h>
#include <stdarg.h>

typedef struct SemaTypeAlias SemaTypeAlias;
typedef size_t SemaTypeId;
typedef struct SemaType SemaType;

bool sema_type_eq(SemaType *a, SemaType *b);
IrTypeId sema_type_ir_id(SemaType *type);
SemaType *sema_type_resolve(SemaType *type);
SemaDecl *sema_type_search_ext(SemaModule *module, SemaType *type, Slice name);

void sema_type_print(va_list list);
