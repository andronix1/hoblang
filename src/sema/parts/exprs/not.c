#include "../../parts.h"

SemaValue *sema_analyze_expr_not(SemaModule *sema, AstExpr *not_expr, SemaExprCtx ctx) { 
    SemaType *type = sema_value_expr_type(sema, not_expr, sema_expr_ctx_expect(ctx, &primitives[PRIMITIVE_BOOL]));
    if (!sema_types_equals(type, &primitives[PRIMITIVE_BOOL])) {
        sema_err("not operator can be only be applied to booleans, not {sema::type}", type);
    }
    return sema_value_const(&primitives[PRIMITIVE_BOOL]);
}
