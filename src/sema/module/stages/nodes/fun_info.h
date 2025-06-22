#pragma once

#include "ast/api/fun.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"

bool sema_func_info_setup(SemaModule *module, AstFunInfo *info);
SemaType *sema_func_info_type(SemaModule *module, AstFunInfo *info);
void sema_module_push_fun_info_decl(SemaModule *module, AstFunInfo *info, SemaValue *value);
