#pragma once

#include "ast/stmt.h"
#include "sema/module/api/module.h"
#include <stdbool.h>

bool sema_module_emit_stmt_break(SemaModule *module, AstLoopControl *control);
bool sema_module_emit_stmt_continue(SemaModule *module, AstLoopControl *control);
