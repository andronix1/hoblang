#pragma once

#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_string(SemaModule *module, Slice string_slice, Slice string, SemaExprCtx ctx);


