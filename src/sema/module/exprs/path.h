#pragma once

#include "ast/api/path.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx);


