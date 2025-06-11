#pragma once

#include "ast/stmt.h"
#include "sema/module/api/module.h"
#include <stdbool.h>

bool sema_module_emit_stmt_assign(SemaModule *module, AstAssign *assign);
