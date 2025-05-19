#pragma once

#include "sema/module/api.h"
#include "sema/module/api/decl_handle.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/type.h"

typedef struct SemaValue SemaValue;

SemaType *sema_value_is_var(SemaValue *value);
SemaType *sema_value_is_runtime(SemaValue *value);
SemaModule *sema_value_is_module(SemaValue *value);
SemaType *sema_value_is_type(SemaValue *value);
SemaDeclHandle *sema_value_is_ext(SemaValue *value);
SemaGeneric *sema_value_is_generic(SemaValue *value);
