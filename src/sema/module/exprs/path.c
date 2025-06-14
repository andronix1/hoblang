#include "path.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/vec.h"
#include "ir/stmt/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/decl.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/type.h"
#include "sema/module/module.h"
#include "sema/module/value.h"
#include <stdio.h>

static SemaValue *sema_module_analyze_expr_path_deref(SemaModule *module, SemaValue *value, Slice slice, SemaExprOutput *output) {
    SemaValueRuntime *runtime = NOT_NULL(sema_value_should_be_runtime(module, slice, value));
    if (runtime->type->kind != SEMA_TYPE_POINTER) {
        sema_module_err(module, slice, "cannot deref non-pointer type $t", runtime->type);
        return NULL;
    }
    size_t step_id = sema_expr_output_push_step(output,
        ir_expr_step_new_deref(sema_module_expr_emit_runtime(runtime, output)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_VAR, runtime->type->pointer_to, step_id);
}

static SemaValue *sema_module_analyze_expr_path_ident(SemaModule *module, SemaValue *value, Slice ident, SemaExprOutput *output) {
    SemaModule *in_module = sema_value_is_module(value);
    if (in_module) {
        return NOT_NULL(sema_module_resolve_req_decl_from(in_module, module, ident))->value;
    }
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (runtime) {
        size_t of = sema_module_expr_emit_runtime(runtime, output);
        SemaType *type = sema_type_resolve(runtime->type);
        if (type->kind == SEMA_TYPE_STRUCTURE) {
            size_t idx = keymap_get_idx(type->structure.fields_map, ident);
            if (idx != (size_t)-1) {
                size_t step_id = sema_expr_output_push_step(output, ir_expr_step_new_struct_field(idx, of));
                keymap_at(type->structure.fields_map, idx, field);
                return sema_value_new_runtime_expr_step(module->mempool, runtime->kind, field->value.type, step_id);
            }
        }
        SemaDecl *decl = sema_type_search_ext(module, runtime->type, ident);
        if (decl) {
            SemaValueRuntime *decl_runtime = sema_value_is_runtime(decl->value);
            if (decl_runtime) {
                size_t step_id = sema_module_expr_emit_runtime(decl_runtime, output);
                return sema_value_new_runtime_ext_expr_step(module->mempool, decl_runtime->kind,
                    decl_runtime->type, step_id, of);
            }
        }
    }
    sema_module_err(module, ident, "cannot find `$S` in $v", ident, value);
    return NULL;
}

SemaValue *sema_module_emit_expr_path_from(SemaModule *module, SemaValue *value, AstPath *path, size_t offset, SemaExprCtx ctx) {
    for (size_t i = offset; i < vec_len(path->segments); i++) {
        AstPathSegment *segment = &path->segments[i];
        switch (segment->kind) {
            case AST_PATH_SEGMENT_IDENT:
                value = NOT_NULL(sema_module_analyze_expr_path_ident(module, value, segment->ident, ctx.output));
                break;
            case AST_PATH_SEGMENT_DEREF:
                value = NOT_NULL(sema_module_analyze_expr_path_deref(module, value, segment->slice, ctx.output));
                break;
            case AST_PATH_SEGMENT_GENERIC_BUILD:
                TODO;
        }
    }
    return value;
}

SemaValue *sema_module_emit_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx) {
    assert(vec_len(path->segments) > 0);
    SemaValue *value = NOT_NULL(sema_module_path_segment(module, &path->segments[0]));
    return sema_module_emit_expr_path_from(module, value, path, 1, ctx);
}


