#pragma once

#include "ast/api/type.h"
#include "sema/module/api/module.h"
#include "sema/module/api/type.h"

SemaType *sema_module_type(SemaModule *module, AstType *type);
SemaType *sema_module_opt_type(SemaModule *module, AstType *type);
