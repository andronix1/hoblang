#include "string.h"
#include "core/null.h"
#include "ir/stmt/expr.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/internal.h"
#include "sema/module/module.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_string(SemaModule *module, Slice string_slice, Slice string, SemaExprCtx ctx) {
    IrDeclId decl_id = *NOT_NULL(sema_module_internal_string_new(module, string_slice));
    size_t str = sema_expr_output_push_step(ctx.output, ir_expr_step_new_string(string));
    size_t length = sema_expr_output_push_step(ctx.output, ir_expr_step_new_int(sema_type_ir_id(module->internal.usize),
        string.length));
    size_t callable = sema_expr_output_push_step(ctx.output, ir_expr_step_new_get_decl(decl_id));
    size_t step_id = sema_expr_output_push_step(ctx.output, ir_expr_step_new_call(vec_create_in(module->mempool,
        str, length), callable));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, module->internal.string.type, step_id);
}


