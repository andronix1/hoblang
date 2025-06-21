#pragma once

#include "core/slice.h"
#include "hir/api/type.h"
#include "hir/api/type.h"
#include "sema/module/api/decl.h"
#include "sema/module/api/module.h"
#include <stddef.h>
#include <stdarg.h>

typedef struct SemaTypeAlias SemaTypeAlias;
typedef size_t SemaTypeId;
typedef struct SemaType SemaType;

SemaType *sema_type_generate(SemaModule *module, SemaType *source, SemaType **params, SemaType **input);
bool sema_type_eq(const SemaType *a, const SemaType *b);

/*
    With:
        type Fd = i32;
        type WinFd = i32;
    Valid:
      - Fd = i32 # Upcast
      - i32 = Fd # Downcast
      - WinFd = i32 = Fd # Downcast -> Upcast
    Invalid:
      - Fd = WinFd
      - WinFd = Fd
*/

typedef enum {
    SEMA_TYPE_CAST_NONE,
    SEMA_TYPE_CAST_UP,
    SEMA_TYPE_CAST_DOWN,
} SemaTypeCastKind;

bool sema_type_can_be_downcasted(SemaType *type, SemaType *to);
bool sema_type_can_be_casted(SemaType *type, SemaType *to);

HirTypeId sema_type_hir_id(SemaType *type);
SemaType *sema_type_root(SemaType *type);
SemaDecl *sema_type_search_ext(SemaModule *module, SemaType *type, Slice name);

void sema_type_print(va_list list);
