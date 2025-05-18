#pragma once

#include "ast/api/stmt.h"
#include "sema/module/api.h"

bool sema_module_analyze_if(SemaModule *module, AstIf *if_else);
