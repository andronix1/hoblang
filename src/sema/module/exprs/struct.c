#include "struct.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/ast/type.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "ast/type.h"
#include "sema/module/value.h"

SemaValue *sema_module_emit_expr_struct(SemaModule *module, AstExprStructConstructor *structure, SemaExprCtx ctx) {
    SemaType *type = sema_type_resolve(NOT_NULL(sema_module_type(module, structure->type)));
    if (type->kind != SEMA_TYPE_STRUCTURE) {
        sema_module_err(module, structure->type->slice, "type is not a structure");
        return NULL;
    }
    size_t *fields = vec_new_in(module->mempool, size_t);
    vec_resize(fields, vec_len(type->structure.fields_map));
    for (size_t i = 0; i < vec_len(fields); i++) {
        fields[i] = -1;
    }
    for (size_t i = 0; i < vec_len(structure->fields_map); i++) {
        keymap_at(structure->fields_map, i, field);
        size_t idx = keymap_get_idx(type->structure.fields_map, field->key);
        if (idx != (size_t)-1) {
            keymap_at(type->structure.fields_map, idx, tfield);
            SemaType *field_type = tfield->value.type;
            SemaValueRuntime *runtime = sema_module_emit_runtime_expr(module, field->value.expr, sema_expr_ctx_new(
                ctx.output, field_type));
            if (!runtime) continue;
            if (!sema_type_eq(runtime->type, field_type)) {
                sema_module_err(module, field->value.expr->slice,
                    "cannot set expression of type $t to field struct of type $t", runtime->type, field_type);
            }
            fields[i] = sema_module_expr_emit_runtime(module->mempool, runtime, ctx.output);
        } else {
            sema_module_err(module, field->key, "there is no such field in specified struct");
        }
    }
    if (vec_len(structure->fields_map) != vec_len(type->structure.fields_map)) {
        sema_module_err(module, structure->type->slice, "there is $l fields in structure, but only $l were initialized",
                vec_len(type->structure.fields_map), vec_len(structure->fields_map));
    }
    size_t step_id = sema_expr_output_push_step(ctx.output, ir_expr_step_new_build_struct(sema_type_ir_id(type), fields));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
}


