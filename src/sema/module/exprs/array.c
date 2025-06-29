#include "array.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_array(SemaModule *module, AstExprArray *array, Slice where, SemaExprCtx ctx) {
    SemaType *type = NULL;
    if (array->type) {
        type = NOT_NULL(sema_module_type(module, array->type));
    } else if (ctx.expectation) {
        type = ctx.expectation;
    } else {
        sema_module_err(module, where, "implicit type with no type expected");
        return NULL;
    }
    size_t *elements = vec_new_in(module->mempool, size_t);
    for (size_t i = 0; i < vec_len(array->elements); i++) {
        SemaValueRuntime *runtime = NOT_NULL(sema_module_emit_runtime_expr(module, array->elements[i],
            sema_expr_ctx_new(ctx.output, type)));
        if (!sema_type_can_be_downcasted(runtime->type, type)) {
            sema_module_err(module, array->elements[i]->slice,
                "expects elements of type $t but expression of type $t passed", type, runtime->type);
        }
        vec_push(elements, sema_module_expr_emit_runtime(module, runtime, ctx.output));
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_array(sema_type_to_hir(module, type), elements));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL,
        sema_type_new_array(module->mempool, vec_len(elements), type), step_id);
}


