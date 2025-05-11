#pragma once

#include "ast/api/type.h"
#include "sema/module/api.h"
#include "sema/module/api/type.h"

SemaType *sema_module_analyze_type(SemaModule *module, AstType *type);
