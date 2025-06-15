#pragma once

#include "ast/node.h"
#include "sema/module/api/module.h"

bool sema_module_stage_init_type_decl(SemaModule *module, AstTypeDecl *type_decl);
bool sema_module_stage_fill_type_decl(SemaModule *module, AstTypeDecl *type_decl);
