#pragma once

#include "ast/stmt.h"
#include "sema/module/api/module.h"
#include <stdbool.h>

bool sema_module_emit_if(SemaModule *module, AstIf *if_else);
