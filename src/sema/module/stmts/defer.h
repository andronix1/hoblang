#pragma once

#include "ast/api/body.h"
#include "sema/module/api/module.h"
#include <stdbool.h>

bool sema_module_emit_stmt_defer(SemaModule *module, AstBody *defer);
