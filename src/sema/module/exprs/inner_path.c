#include "ast/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/exprs/path.h"

SemaValue *sema_module_emit_expr_inner_path(SemaModule *module, AstExprInnerPath *inner_path, SemaExprCtx ctx) {
    SemaValue *value = sema_module_emit_expr(module, inner_path->inner, sema_expr_ctx_new(ctx.output, NULL));
    return sema_module_emit_expr_path_from(module, value, inner_path->path, 0, sema_expr_ctx_new(ctx.output, NULL));
}


