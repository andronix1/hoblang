#pragma once

#include "ast/api/expr.h"
#include "sema/module/api.h"
#include "sema/module/api/type.h"

typedef struct {

} SemaExprCtx;

SemaType *sema_module_analyze_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx);
SemaExprCtx sema_expr_ctx_new();
