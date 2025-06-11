#pragma once

#include "ast/expr.h"
#include "sema/module/api/module.h"

bool sema_module_emit_stmt_expr(SemaModule *module, AstExpr *expr);

