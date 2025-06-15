#pragma once

#include "ast/node.h"
#include "sema/module/api/module.h"

bool sema_module_stage_fill_fun(SemaModule *module, AstFunDecl *fun_decl);
bool sema_module_stage_emit_fun(SemaModule *module, AstFunDecl *fun_decl);
