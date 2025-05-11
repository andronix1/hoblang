#pragma once

#include "sema/module/api/type.h"

typedef struct SemaValue SemaValue;

SemaType *sema_value_is_type(SemaValue *value);
