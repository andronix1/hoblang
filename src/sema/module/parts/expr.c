#include "ast/private/expr/anon_fun.h"
#include "core/location.h"
#include "sema/const/const.h"
#include "sema/module/private.h"
#include "sema/module/parts/expr.h"
#include "sema/type/private.h"
#include "sema/value/private.h"
#include "sema/value/api.h"
#include "sema/const/api.h"
#include "ast/private/expr.h"
#include "core/assert.h"

SemaValue *sema_analyze_expr_anon_fun(SemaModule *sema, FileLocation loc, AstExprAnonFun *anon_fun, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_array(SemaModule *sema, AstExpr **array, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_as(SemaModule *sema, AstExprAs *as, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_binop(SemaModule *sema, AstExprBinop *binop, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_call(SemaModule *sema, AstCall *call, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_int(SemaModule *sema, uint64_t integer, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_float(SemaModule *sema, long double float_value, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_ref(SemaModule *sema, AstExpr *ref_expr, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_not(SemaModule *sema, AstExpr *not_expr, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_unary(SemaModule *sema, AstExprUnary *unary, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_idx(SemaModule *sema, AstExprIdx *idx, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_null(SemaModule *sema, SemaNullType *type, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_get_inner(SemaModule *sema, AstExprGetInner *get_inner, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_get_local(SemaModule *sema, AstExprGetLocal *get_local, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_ret_on_null(SemaModule *sema, AstExprRetOnNull *ret_on_null, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_unwrap(SemaModule *sema, AstExprUnwrap *unwrap, SemaExprCtx ctx);
SemaValue *sema_analyze_expr_struct(SemaModule *sema, FileLocation at, AstExprStruct *structure, SemaExprCtx ctx);

SemaValue *sema_expr(SemaModule *sema, AstExpr *expr, SemaExprCtx ctx) {
    ctx.loc = expr->loc;
    switch (expr->type) {
		case AST_EXPR_IDX: return expr->value = sema_analyze_expr_idx(sema, &expr->idx, ctx);
		case AST_EXPR_STRUCT: return expr->value = sema_analyze_expr_struct(sema, expr->loc, &expr->structure, ctx);
		case AST_EXPR_ANON_FUN: return expr->value = sema_analyze_expr_anon_fun(sema, expr->loc, &expr->anon_fun, ctx);
		case AST_EXPR_UNWRAP: return expr->value = sema_analyze_expr_unwrap(sema, &expr->unwrap, ctx);
		case AST_EXPR_RET_ON_NULL: return expr->value = sema_analyze_expr_ret_on_null(sema, &expr->ret_on_null, ctx);
		case AST_EXPR_UNARY: return expr->value = sema_analyze_expr_unary(sema, &expr->unary, ctx);
		case AST_EXPR_GET_INNER_PATH: return expr->value = sema_analyze_expr_get_inner(sema, &expr->get_inner, ctx);
		case AST_EXPR_GET_LOCAL_PATH: return expr->value = sema_analyze_expr_get_local(sema, &expr->get_local, ctx);
		case AST_EXPR_REF: return expr->value = sema_analyze_expr_ref(sema, expr->ref_expr, ctx);
		case AST_EXPR_NOT: return expr->value = sema_analyze_expr_not(sema, expr->not_expr, ctx);
        // TODO: strings - arrays
		case AST_EXPR_STR: return expr->value = sema_value_final(sema_type_new_slice(sema_type_primitive_u8()));
		case AST_EXPR_CHAR: return expr->value = sema_value_const(
            sema_const_int(sema_type_primitive_u8(), expr->character)
        );
		case AST_EXPR_BOOL: return expr->value = sema_value_const(sema_const_bool(expr->boolean));
		case AST_EXPR_ARRAY: return expr->value = sema_analyze_expr_array(sema, expr->array, ctx);
		case AST_EXPR_BINOP: return expr->value = sema_analyze_expr_binop(sema, &expr->binop, ctx);
		case AST_EXPR_AS: return expr->value = sema_analyze_expr_as(sema, &expr->as, ctx);
		case AST_EXPR_NULL: return expr->value = sema_analyze_expr_null(sema, &expr->null_type, ctx);
		case AST_EXPR_INTEGER: return expr->value = sema_analyze_expr_int(sema, expr->integer, ctx);
		case AST_EXPR_FLOAT: return expr->value = sema_analyze_expr_float(sema, expr->float_value, ctx);
        case AST_EXPR_CALL: return expr->value = sema_analyze_expr_call(sema, &expr->call, ctx);
    }
	assert(0, "invalid ast expr type: {int}", expr->type);
}

SemaValue *sema_callable_expr_type(SemaModule *sema, AstExpr *expr, SemaExprCtx ctx) {
    if (!sema_expr(sema, expr, ctx)) {
        return NULL;
    }
    if (
        expr->value->type != SEMA_VALUE_FINAL &&
        expr->value->type != SEMA_VALUE_VAR &&
        expr->value->type != SEMA_VALUE_EXT_FUNC_HANDLE) {
        SEMA_ERROR(expr->loc, "{ast::expr} is not callable", expr);
        return NULL;
    }
    return expr->value;
}

SemaConst *sema_const_expr(SemaModule *sema, AstExpr *expr, SemaExprCtx ctx) {
    if (!sema_expr(sema, expr, ctx)) {
        return NULL;
    }
    if (expr->value->type != SEMA_VALUE_CONST) {
        SEMA_ERROR(expr->loc, "{ast::expr} is not a const", expr);
        return NULL;
    }
    return &expr->value->constant;
}

SemaType *sema_var_expr_type(SemaModule *sema, AstExpr *expr, SemaExprCtx ctx) {
    if (!sema_expr(sema, expr, ctx)) {
        return NULL;
    }
    if (expr->value->type != SEMA_VALUE_VAR) {
        SEMA_ERROR(expr->loc, "{ast::expr} is not a variable", expr);
    }
    return sema_value_typeof(expr->value);
}

SemaType *sema_value_expr_type(SemaModule *sema, AstExpr *expr, SemaExprCtx ctx) {
    if (!sema_expr(sema, expr, ctx)) {
        return NULL;
    }
    if (expr->value->type != SEMA_VALUE_CONST && expr->value->type != SEMA_VALUE_FINAL && expr->value->type != SEMA_VALUE_VAR) {
        SEMA_ERROR(expr->loc, "{ast::expr} is not a value", expr);
        return NULL;
    }
    return sema_value_typeof(expr->value);
}
