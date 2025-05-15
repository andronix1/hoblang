#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/path.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

static inline SemaValue *_sema_module_analyze_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    switch (expr->kind) {
        case AST_EXPR_PATH:
            return sema_module_resolve_path(module, expr->path);
        case AST_EXPR_INTEGER:
            // TODO: module arch + const
            if (ctx.expecting->kind == SEMA_TYPE_PRIMITIVE && ctx.expecting->primitive.kind == SEMA_PRIMITIVE_INT) {
                return sema_value_new_final(module->mempool, ctx.expecting);
            }
            return sema_value_new_final(module->mempool,
                sema_type_new_primitive_int(module->mempool, SEMA_PRIMITIVE_INT32, true));
        case AST_EXPR_CALL: {
            SemaType *type = sema_value_is_runtime(NOT_NULL(
                sema_module_analyze_expr(module, expr->call.inner, sema_expr_ctx_new(NULL))));
            if (!type) {
                sema_module_err(module, expr->slice, "this is not runtime value");
                return NULL;
            }
            if (type->kind != SEMA_TYPE_FUNCTION) {
                sema_module_err(module, expr->slice, "this is not function");
                return NULL;
            }
            size_t len = vec_len(type->function.args);
            size_t passed = vec_len(expr->call.args);
            if (passed != len) {
                if (passed < len) {
                    len = passed;
                }
                sema_module_err(module, expr->slice, "expected $l arguments but passed $l", len, passed);
            }
            for (size_t i = 0; i < len; i++) {
                SemaType *expects = type->function.args[i];
                AstExpr *arg = expr->call.args[i];
                SemaValue *value = sema_module_analyze_expr(module, arg, sema_expr_ctx_new(expects));
                if (!value) {
                    continue;
                }
                SemaType *arg_type = sema_value_is_runtime(value);
                if (!arg_type) {
                    sema_module_err(module, arg->slice, "argument must be runtime value");
                    continue;
                }
                if (!sema_type_eq(expects, arg_type)) {
                    sema_module_err(module, arg->slice, "expected $t but got $t", expects, arg_type);
                    continue;
                }
            }
            return sema_value_new_final(module->mempool, type->function.returns);
        }
        case AST_EXPR_SCOPE:
            return sema_module_analyze_expr(module, expr->scope, ctx);
        case AST_EXPR_BINOP: {
            SemaType *left = sema_value_is_runtime(NOT_NULL(sema_module_analyze_expr(module, expr->binop.left, ctx)));
            if (!left) {
                sema_module_err(module, expr->binop.left->slice, "binop side expression must be runtime value");
                return NULL;
            }
            SemaType *right = sema_value_is_runtime(NOT_NULL(sema_module_analyze_expr(module, expr->binop.right, ctx)));
            if (!right) {
                sema_module_err(module, expr->binop.right->slice, "binop side expression must be runtime value");
                return NULL;
            }
            if (!sema_type_eq(left, right)) {
                sema_module_err(module, expr->slice, "binop can operate only expressions with equal type");
                return NULL;
            }
            static bool logged = false;
            if (!logged) {
                logln("WARNING: binops are unstable! use them carefully!");
                logged = true;
            }
            return sema_value_new_final(module->mempool, left);
        }
        case AST_EXPR_STRING:
            return sema_value_new_final(module->mempool, sema_type_new_slice(module->mempool,
                sema_type_new_primitive_int(module->mempool, SEMA_PRIMITIVE_INT8, false)));
    }
    UNREACHABLE;
}

SemaValue *sema_module_analyze_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    return expr->sema.value = _sema_module_analyze_expr(module, expr, ctx);
}

SemaExprCtx sema_expr_ctx_new(SemaType *expecting) {
    SemaExprCtx ctx = {
        .expecting = expecting
    };
    return ctx;
}
