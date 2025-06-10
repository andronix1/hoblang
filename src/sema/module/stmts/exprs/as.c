#include "as.h"
#include "core/null.h"
#include "ir/stmt/expr.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_analyze_expr_as(SemaModule *module, AstAs *as, SemaExprCtx ctx) {
    SemaType *dest = NOT_NULL(sema_module_type(module, as->type));
    SemaValueRuntime *source_runtime = NOT_NULL(sema_module_analyze_runtime_expr(module, as->inner,
        sema_expr_ctx_new(ctx.output, NULL)));
    SemaType *source = source_runtime->type;

    if (source->kind == SEMA_TYPE_INT && dest->kind == SEMA_TYPE_INT) {
        vec_push(ctx.output->steps, ir_expr_step_new_cast_int(sema_module_expr_get_runtime(source_runtime, ctx.output),
            sema_type_ir_id(source), sema_type_ir_id(dest)));
    } else {
        sema_module_err(module, as->slice, "$t cannot be casted to $t", source, dest);
    }

    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, dest, vec_len(ctx.output->steps) - 1);
}
