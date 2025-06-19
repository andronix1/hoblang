#include "expr.h"
#include "ast/expr.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/array.h"
#include "sema/module/exprs/as.h"
#include "sema/module/exprs/binop.h"
#include "sema/module/exprs/bool.h"
#include "sema/module/exprs/call.h"
#include "sema/module/exprs/char.h"
#include "sema/module/exprs/float.h"
#include "sema/module/exprs/function.h"
#include "sema/module/exprs/idx.h"
#include "sema/module/exprs/int.h"
#include "sema/module/exprs/neg.h"
#include "sema/module/exprs/not.h"
#include "sema/module/exprs/path.h"
#include "sema/module/exprs/inner_path.h"
#include "sema/module/exprs/string.h"
#include "sema/module/exprs/struct.h"
#include "sema/module/exprs/take_ref.h"
#include "sema/module/module.h"
#include "sema/module/value.h"
#include <assert.h>

SemaValue *sema_module_emit_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    switch (expr->kind) {
        case AST_EXPR_PATH: return sema_module_emit_expr_path(module, expr->path, ctx);
        case AST_EXPR_INTEGER: return sema_module_emit_expr_int(module, expr->integer, ctx);
        case AST_EXPR_FLOAT: return sema_module_emit_expr_float(module, expr->float_value, ctx);
        case AST_EXPR_CALL: return sema_module_emit_expr_call(module, &expr->call, ctx);
        case AST_EXPR_SCOPE: return sema_module_emit_expr(module, expr->scope, ctx);
        case AST_EXPR_CHAR: return sema_module_emit_expr_char(module, expr->character, ctx);
        case AST_EXPR_BOOL: return sema_module_emit_expr_bool(module, expr->boolean, ctx);
        case AST_EXPR_BINOP: return sema_module_emit_expr_binop(module, &expr->binop, ctx);
        case AST_EXPR_AS: return sema_module_emit_expr_as(module, &expr->as, ctx);
        case AST_EXPR_NOT: return sema_module_emit_expr_not(module, expr->not_inner, ctx);
        case AST_EXPR_NEG: return sema_module_emit_expr_neg(module, expr->neg_inner, ctx);
        case AST_EXPR_TAKE_REF: return sema_module_emit_expr_take_ref(module, expr->not_inner, ctx);
        case AST_EXPR_INNER_PATH: return sema_module_emit_expr_inner_path(module, &expr->inner_path, ctx);
        case AST_EXPR_STRUCT: return sema_module_emit_expr_struct(module, &expr->structure, ctx);
        case AST_EXPR_STRING: return sema_module_emit_expr_string(module, expr->slice, expr->string, ctx);
        case AST_EXPR_ARRAY: return sema_module_emit_expr_array(module, &expr->array, ctx);
        case AST_EXPR_IDX: return sema_module_emit_expr_idx(module, &expr->idx, ctx);
        case AST_EXPR_FUNCTION: return sema_module_emit_expr_function(module, &expr->func);
    }
    UNREACHABLE;
}

size_t sema_module_expr_emit_runtime(SemaValueRuntime *runtime, SemaExprOutput *output) {
    switch (runtime->val_kind) {
        case SEMA_VALUE_RUNTIME_GLOBAL:
            return sema_expr_output_push_step(output, hir_expr_step_new_get_decl(runtime->global_id));
        case SEMA_VALUE_RUNTIME_LOCAL:
            return sema_expr_output_push_step(output, hir_expr_step_new_get_local(runtime->global_id));
        case SEMA_VALUE_RUNTIME_CONST:
            return sema_expr_output_push_step(output, hir_expr_step_new_const(sema_const_to_hir(runtime->constant)));
        case SEMA_VALUE_RUNTIME_EXPR_STEP:
            return runtime->in_expr_id.step_id;
    }
    UNREACHABLE;
}

size_t sema_expr_output_push_step(SemaExprOutput *output, HirExprStep step) {
    if (!output) return 0;
    vec_push(output->steps, step);
    return vec_len(output->steps) - 1;
}

size_t sema_expr_output_last_id(SemaExprOutput *output) {
    if (!output) return 0;
    return vec_len(output->steps) - 1;
}

HirExpr sema_expr_output_collect(SemaExprOutput *output) {
    assert(output);
    return hir_expr_new(output->steps);
}

SemaValueRuntime *sema_module_emit_runtime_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx) {
    SemaValue *value = NOT_NULL(sema_module_emit_expr(module, expr, ctx)); 
    SemaValueRuntime *runtime = NOT_NULL(sema_value_should_be_runtime(module, expr->slice, value));
    sema_module_expr_emit_runtime(runtime, ctx.output);
    return runtime;
}
