#pragma once

#include "ast/node.h"
#include "sema/module/api/module.h"

bool sema_module_stage_fill_value(SemaModule *module, AstValueDecl *value_decl);
