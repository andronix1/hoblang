#pragma once

#include "ast/api/fun.h"
#include "sema/module/api/type.h"

SemaType *sema_func_info_type(SemaModule *module, AstFunInfo *info);
void sema_module_push_fun_info_decl(SemaModule *module, AstFunInfo *info, SemaDecl *decl);
