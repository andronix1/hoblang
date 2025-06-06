#pragma once

#include "ast/stmt.h"
#include "sema/module/api/module.h"

bool sema_module_emit_stmt(SemaModule *module, AstStmt *stmt);
