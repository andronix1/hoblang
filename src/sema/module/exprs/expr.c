#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/as.h"
#include "sema/module/exprs/binop.h"
#include "sema/module/exprs/bool.h"
#include "sema/module/exprs/call.h"
#include "sema/module/exprs/char.h"
#include "sema/module/exprs/int.h"
#include "sema/module/exprs/not.h"
#include "sema/module/exprs/path.h"
#include "sema/module/exprs/take_ref.h"
#include "sema/module/value.h"
#include <stdio.h>

SemaValue *sema_module_emit_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    switch (expr->kind) {
        case AST_EXPR_PATH: return sema_module_emit_expr_path(module, expr->path, ctx);
        case AST_EXPR_INTEGER: return sema_module_emit_expr_int(module, expr->integer, ctx);
        case AST_EXPR_CALL: return sema_module_emit_expr_call(module, &expr->call, ctx);
        case AST_EXPR_SCOPE: return sema_module_emit_expr(module, expr->scope, ctx);
        case AST_EXPR_CHAR: return sema_module_emit_expr_char(module, expr->character, ctx);
        case AST_EXPR_BOOL: return sema_module_emit_expr_bool(module, expr->boolean, ctx);
        case AST_EXPR_BINOP: return sema_module_emit_expr_binop(module, &expr->binop, ctx);
        case AST_EXPR_AS: return sema_module_emit_expr_as(module, &expr->as, ctx);
        case AST_EXPR_NOT: return sema_module_emit_expr_not(module, expr->not_inner, ctx);
        case AST_EXPR_TAKE_REF: return sema_module_emit_expr_take_ref(module, expr->not_inner, ctx);
        case AST_EXPR_STRING: case AST_EXPR_STRUCT: TODO;
    }
    UNREACHABLE;
}

size_t sema_module_expr_emit_runtime(SemaValueRuntime *runtime, SemaExprOutput *output) {
    size_t step_id = vec_len(output->steps);
    switch (runtime->val_kind) {
        case SEMA_VALUE_RUNTIME_GLOBAL:
            vec_push(output->steps, ir_expr_step_new_get_decl(runtime->global_id));
            return step_id;
        case SEMA_VALUE_RUNTIME_LOCAL:
            vec_push(output->steps, ir_expr_step_new_get_local(runtime->local_id));
            return step_id;
        case SEMA_VALUE_RUNTIME_EXPR_STEP:
            return runtime->in_expr_id.step_id;
    }
    UNREACHABLE;
}

SemaValueRuntime *sema_module_emit_runtime_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    SemaValue *value = NOT_NULL(sema_module_emit_expr(module, expr, ctx)); 
    return NOT_NULL(sema_value_should_be_runtime(module, expr->slice, value));
}
