#pragma once

#include "ast/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/stmts/expr.h"

SemaValue *sema_module_analyze_expr_as(SemaModule *module, AstAs *as, SemaExprCtx ctx);

