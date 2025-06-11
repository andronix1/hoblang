#pragma once

#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_not(SemaModule *module, AstExpr *inner, SemaExprCtx ctx);


