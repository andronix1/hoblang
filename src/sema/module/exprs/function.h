#pragma once

#include "ast/expr.h"
#include "sema/module/api/value.h"

SemaValue *sema_module_emit_expr_function(SemaModule *module, AstExprFunc *func);


