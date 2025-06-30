#include "struct.h"
#include "core/mempool.h"
#include "core/null.h"
#include "sema/module/api/const.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/type.h"
#include "sema/module/const.h"
#include "sema/module/module.h"
#include "sema/module/type.h"
#include "ast/type.h"
#include "sema/module/value.h"
#include <string.h>

SemaValue *sema_module_emit_expr_struct(SemaModule *module, AstExprStructConstructor *structure, Slice where, SemaExprCtx ctx) {
    SemaType *type = NULL;
    Slice type_slice = where;
    if (structure->type) {
        type = NOT_NULL(sema_module_type(module, structure->type));
        type_slice = structure->type->slice;
    } else if (ctx.expectation) {
        type = ctx.expectation;
    } else {
        sema_module_err(module, where, "implicit type with no type expected");
        return NULL;
    }
    SemaType *root = sema_type_root(type);
    if (root->kind != SEMA_TYPE_STRUCTURE) {
        sema_module_err(module, type_slice, "type is not a structure");
        return NULL;
    }

    size_t fields_count = vec_len(root->structure.fields_map);

    size_t *fields = vec_new_in(module->mempool, size_t);
    vec_resize(fields, fields_count);

    SemaConst **constants = vec_new_in(module->mempool, SemaConst*);
    vec_resize(constants, fields_count);

    bool is_not_const = false;

    for (size_t i = 0; i < vec_len(structure->fields_map); i++) {
        keymap_at(structure->fields_map, i, field);
        size_t idx = keymap_get_idx(root->structure.fields_map, field->key);
        if (idx != (size_t)-1) {
            keymap_at(root->structure.fields_map, idx, tfield);

            SemaType *field_type = tfield->value.type;
            SemaValueRuntime *runtime = sema_module_emit_runtime_expr(module, field->value.expr, sema_expr_ctx_new(
                ctx.output, field_type));
            if (!runtime) continue;
            if (!sema_type_can_be_downcasted(runtime->type, field_type)) {
                sema_module_err(module, field->value.expr->slice,
                    "cannot set expression of type $t to field struct of type $t", runtime->type, field_type);
                continue;
            }

            SemaConst *constant = sema_value_runtime_is_const(runtime);
            constants[idx] = constant;
            if (!constant) is_not_const = true;
            fields[idx] = sema_module_expr_emit_runtime(module, runtime, ctx.output);
        } else {
            sema_module_err(module, field->key, "there is no such field in specified struct");
        }
    }
    if (vec_len(structure->fields_map) != vec_len(root->structure.fields_map)) {
        sema_module_err(module, type_slice, "there is $l fields in structure, but only $l were initialized",
                vec_len(root->structure.fields_map), vec_len(structure->fields_map));
    }
    if (is_not_const) {
        size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_build_struct(sema_type_to_hir(module, root), fields));
        return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, type, step_id);
    } else {
        return sema_value_new_runtime_const(module->mempool, sema_const_new_struct(module->mempool, type, constants));
    }
}


