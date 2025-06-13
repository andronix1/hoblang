#pragma once

#include "ast/api/path.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

SemaValue *sema_module_emit_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx);
SemaValue *sema_module_emit_expr_path_from(SemaModule *module, SemaValue *value, AstPath *path, size_t offset, SemaExprCtx ctx);


