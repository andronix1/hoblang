#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/stmts/exprs/binop.h"
#include "sema/module/stmts/exprs/bool.h"
#include "sema/module/stmts/exprs/call.h"
#include "sema/module/stmts/exprs/char.h"
#include "sema/module/stmts/exprs/int.h"
#include "sema/module/stmts/exprs/path.h"
#include "sema/module/value.h"

SemaValue *sema_module_analyze_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    switch (expr->kind) {
        case AST_EXPR_PATH:
            return sema_module_analyze_expr_path(module, expr->path, ctx);
        case AST_EXPR_INTEGER:
            return sema_module_analyze_expr_int(module, expr->integer, ctx);
        case AST_EXPR_CALL:
            return sema_module_analyze_expr_call(module, &expr->call, ctx);
        case AST_EXPR_SCOPE:
            return sema_module_analyze_expr(module, expr->scope, ctx);
        case AST_EXPR_CHAR:
            return sema_module_analyze_expr_char(module, expr->character, ctx);
        case AST_EXPR_BOOL:
            return sema_module_analyze_expr_bool(module, expr->boolean, ctx);
        case AST_EXPR_BINOP:
            return sema_module_analyze_expr_binop(module, &expr->binop, ctx);
        case AST_EXPR_STRING:
        case AST_EXPR_STRUCT:
            TODO;
          break;
        }
    UNREACHABLE;
}

size_t sema_module_expr_get_runtime(SemaValueRuntime *runtime, SemaExprOutput *output) {
    size_t step_id = vec_len(output->steps);
    switch (runtime->kind) {
        case SEMA_VALUE_RUNTIME_GLOBAL:
            vec_push(output->steps, ir_expr_step_new_get_decl(runtime->global_id));
            return step_id;
        case SEMA_VALUE_RUNTIME_LOCAL:
            vec_push(output->steps, ir_expr_step_new_get_local(runtime->local_id));
            return step_id;
        case SEMA_VALUE_RUNTIME_EXPR_STEP:
            return runtime->step_id;
    }
    UNREACHABLE;
}

SemaValueRuntime *sema_module_analyze_runtime_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    SemaValue *value = NOT_NULL(sema_module_analyze_expr(module, expr, ctx)); 
    return NOT_NULL(sema_value_should_be_runtime(module, expr->slice, value));
}
