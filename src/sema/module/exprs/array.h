#pragma once

#include "ast/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_array(SemaModule *module, AstExprArray *array, Slice where, SemaExprCtx ctx);


