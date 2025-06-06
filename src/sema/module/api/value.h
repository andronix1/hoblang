#pragma once

#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

typedef struct SemaValue SemaValue;

SemaType *sema_value_should_be_type(SemaModule *module, Slice where, SemaValue *value);
SemaType *sema_value_is_type(SemaValue *value);
