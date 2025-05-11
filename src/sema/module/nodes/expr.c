#include "expr.h"
#include "sema/module/module.h"

SemaType *sema_module_analyze_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    return NULL;
}

SemaExprCtx sema_expr_ctx_new() {
    SemaExprCtx ctx = {};
    return ctx;
}
