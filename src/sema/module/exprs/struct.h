#pragma once

#include "ast/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_struct(SemaModule *module, AstExprStructConstructor *structure, Slice where, SemaExprCtx ctx);


