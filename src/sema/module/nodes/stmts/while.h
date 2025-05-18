#pragma once

#include "ast/stmt.h"
#include "sema/module/api.h"

bool sema_module_analyze_while(SemaModule *module, AstWhile *while_loop);
