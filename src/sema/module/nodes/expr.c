#include "expr.h"
#include "ast/expr.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/path.h"
#include "sema/module/nodes/type.h"
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
            SemaValue *value = NOT_NULL(sema_module_analyze_expr(module, expr->call.inner,
                sema_expr_ctx_new(NULL)));
            SemaType *type = sema_value_is_runtime(value);
            if (!type) {
                sema_module_err(module, expr->slice, "this is not runtime value");
                return NULL;
            }
            if (type->kind != SEMA_TYPE_FUNCTION) {
                sema_module_err(module, expr->slice, "this is not function");
                return NULL;
            }
            bool offset = value->runtime.ext != NULL;
            size_t len = vec_len(type->function.args) - offset;
            size_t passed = vec_len(expr->call.args);
            if (passed != len) {
                if (passed < len) {
                    len = passed;
                }
                sema_module_err(module, expr->slice, "expected $l arguments but passed $l", len, passed);
            }
            for (size_t i = 0; i < len; i++) {
                SemaType *expects = type->function.args[i + offset];
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
            // TODO: std.Slice.<u8>
            return sema_value_new_final(module->mempool, sema_type_new_pointer(module->mempool,
                sema_type_new_primitive_int(module->mempool, SEMA_PRIMITIVE_INT8, false)));
        case AST_EXPR_STRUCT: {
            SemaType *type = NOT_NULL(sema_module_analyze_type(module, expr->structure.type));
            if (type->kind != SEMA_TYPE_STRUCT) {
                sema_module_err(module, expr->structure.type->slice, "type must be struct");
            } else {
                for (size_t i = 0; i < vec_len(expr->structure.fields_map); i++) {
                    keymap_at(expr->structure.fields_map, i, field);
                    size_t idx = keymap_get_idx(type->structure.fields_map, field->key);
                    if (idx == -1) {
                        sema_module_err(module, field->key, "there is no field `$S` in structure $t",
                            field->key, type);
                        continue;
                    }
                    field->value.sema.field_idx = idx;
                    keymap_at(type->structure.fields_map, idx, real_field);
                    SemaType *real_type = real_field->value.type;
                    SemaType *got_type = sema_value_is_runtime(NOT_NULL(
                        sema_module_analyze_expr(module, field->value.expr,
                            sema_expr_ctx_new(real_type))));
                    if (!got_type) {
                        sema_module_err(module, field->value.expr->slice,
                            "struct field initializer must be runtime value");
                        continue;
                    }
                    if (!sema_type_eq(real_type, got_type)) {
                        sema_module_err(module, field->value.expr->slice,
                            "type of struct field is $t bot expression has type $t",
                                real_type, got_type);
                    }
                }
            }
            return sema_value_new_final(module->mempool, type);
        }
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
