#pragma once

#include "sema/module/api/value.h"
#include "sema/module/stmts/expr.h"

SemaValue *sema_module_analyze_expr_bool(SemaModule *module, bool val, SemaExprCtx ctx);


