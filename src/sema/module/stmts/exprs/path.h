#pragma once

#include "ast/api/path.h"
#include "sema/module/api/value.h"
#include "sema/module/stmts/expr.h"

SemaValue *sema_module_analyze_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx);


