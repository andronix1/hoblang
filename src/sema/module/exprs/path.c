#include "path.h"
#include "ast/api/generic.h"
#include "ast/generic.h"
#include "ast/path.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/ast/type.h"
#include "sema/module/const.h"
#include "sema/module/decl.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/generic.h"
#include "sema/module/std.h"
#include "sema/module/type.h"
#include "sema/module/module.h"
#include "sema/module/value.h"
#include <stdio.h>

typedef struct {
    struct {
        bool should_be_cancelled;
        bool is;
        size_t of;
    } ext;
} SemaPathCtx;

static inline SemaPathCtx sema_path_ctx_new() {
    SemaPathCtx ctx = {
        .ext = {
            .should_be_cancelled = true,
            .is = false,
        }
    };
    return ctx;
}

static inline void sema_path_ctx_make_ext(SemaPathCtx *ctx, size_t of) {
    ctx->ext.is = true;
    ctx->ext.of = of;
    ctx->ext.should_be_cancelled = false;
}

static inline void sema_path_ctx_continue_ext(SemaPathCtx *ctx) {
    ctx->ext.should_be_cancelled = false;
}

static inline void sema_path_ctx_step(SemaPathCtx *ctx) {
    if (ctx->ext.should_be_cancelled) {
        ctx->ext.is = false;
    }
    ctx->ext.should_be_cancelled = true;
}

static inline SemaValue *sema_path_ctx_finalize_value(SemaModule *module, SemaPathCtx *ctx, SemaValue *value, SemaExprOutput *output) {
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (ctx->ext.is && runtime) {
        return sema_value_new_runtime_ext_expr_step(module->mempool, runtime->kind, runtime->type, 
            sema_module_expr_emit_runtime(module, runtime, output), ctx->ext.of);
    }
    return value;
}

static SemaValue *sema_module_analyze_expr_path_deref(SemaModule *module, SemaValue *value, Slice slice, SemaExprOutput *output) {
    SemaValueRuntime *runtime = NOT_NULL(sema_value_should_be_runtime(module, slice, value));
    if (runtime->type->kind != SEMA_TYPE_POINTER) {
        sema_module_err(module, slice, "cannot deref non-pointer type $t", runtime->type);
        return NULL;
    }
    size_t step_id = sema_expr_output_push_step(output,
        hir_expr_step_new_deref(sema_module_expr_emit_runtime(module, runtime, output)));
    return sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_VAR, runtime->type->pointer_to, step_id);
}

static SemaValue *sema_value_analyze_expr_simple_ident(SemaModule *module, SemaType *root, size_t of, SemaRuntimeKind runtime, Slice ident, SemaExprOutput *output) {
    if (root->kind == SEMA_TYPE_STRUCTURE) {
        size_t idx = keymap_get_idx(root->structure.fields_map, ident);
        if (idx != (size_t)-1) {
            size_t step_id = sema_expr_output_push_step(output, hir_expr_step_new_struct_field(idx, of));
            keymap_at(root->structure.fields_map, idx, field);
            if (field->value.module && field->value.module != module) {
                sema_module_err(module, ident, "field is private");
            }
            return sema_value_new_runtime_expr_step(module->mempool, runtime, field->value.type, step_id);
        }
    }
    if (root->kind == SEMA_TYPE_ARRAY) {
        if (slice_eq(ident, slice_from_cstr("length"))) {
            return sema_value_new_runtime_const(module->mempool, sema_const_new_integer(module->mempool,
                sema_module_std_usize(module, ident), root->array.length));
        }
    }
    return NULL;
}

static SemaValue *sema_module_analyze_expr_path_ident(SemaModule *module, SemaValue *value, Slice ident, SemaExprOutput *output, SemaPathCtx *path_ctx) {
    SemaModule *in_module = sema_value_is_module(value);
    if (in_module) {
        return NOT_NULL(sema_module_resolve_req_decl_from(in_module, module, ident))->value;
    }
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (runtime) {
        size_t of = sema_module_expr_emit_runtime(module, runtime, output);
        SemaType *root = sema_type_root(runtime->type);
        SemaValue *simple_value = sema_value_analyze_expr_simple_ident(module, root, of, runtime->kind, ident, output);
        if (simple_value) {
            return simple_value;
        }
        if (root->kind == SEMA_TYPE_STRUCTURE) {
            size_t idx = keymap_get_idx(root->structure.fields_map, ident);
            if (idx != (size_t)-1) {
                size_t step_id = sema_expr_output_push_step(output, hir_expr_step_new_struct_field(idx, of));
                keymap_at(root->structure.fields_map, idx, field);
                return sema_value_new_runtime_expr_step(module->mempool, runtime->kind, field->value.type, step_id);
            }
        }
        if (root->kind == SEMA_TYPE_ARRAY) {
            if (slice_eq(ident, slice_from_cstr("length"))) {
                return sema_value_new_runtime_const(module->mempool, sema_const_new_integer(module->mempool,
                    sema_module_std_usize(module, ident), root->array.length));
            }
        }
        SemaExtDecl ext;
        if (sema_type_search_ext(module, runtime->type, ident, &ext)) {
            if (ext.by_ref) {
                if (runtime->kind != SEMA_RUNTIME_VAR) {
                    sema_module_err(module, ident, "`$S` is defined for variables $t", ident, runtime->type);
                }
                of = sema_expr_output_push_step(output, hir_expr_step_new_take_ref(of));
            }
            sema_path_ctx_make_ext(path_ctx, of);
            return ext.function;
        }
        if (root->kind == SEMA_TYPE_POINTER) {
            size_t deref_of = sema_expr_output_push_step(output, hir_expr_step_new_deref(of));
            SemaValue *simple_value = sema_value_analyze_expr_simple_ident(module, sema_type_root(root->pointer_to),
                deref_of, SEMA_RUNTIME_VAR, ident, output);
            if (simple_value) {
                return simple_value;
            }
        }
        sema_module_err(module, ident, "cannot find `$S` in value of type $t", ident, runtime->type);
        return NULL;
    }
    SemaType *type = sema_value_is_type(value);
    if (type) {
        SemaExtDecl ext;
        if (sema_type_search_ext(module, type, ident, &ext)) {
            return ext.function;
        }
    }
    sema_module_err(module, ident, "cannot find `$S` in $v", ident, value);
    return NULL;
}

SemaValue *sema_module_emit_expr_path_from(SemaModule *module, SemaValue *value, AstPath *path, size_t offset, SemaExprCtx ctx) {
    SemaPathCtx path_ctx = sema_path_ctx_new();
    for (size_t i = offset; i < vec_len(path->segments); i++) {
        AstPathSegment *segment = &path->segments[i];
        switch (segment->kind) {
            case AST_PATH_SEGMENT_IDENT:
                value = NOT_NULL(sema_module_analyze_expr_path_ident(module, value, segment->ident, ctx.output, &path_ctx));
                break;
            case AST_PATH_SEGMENT_DEREF:
                value = NOT_NULL(sema_module_analyze_expr_path_deref(module, value, segment->slice, ctx.output));
                break;
            case AST_PATH_SEGMENT_SIZEOF: {
                SemaType *type = sema_value_is_type(value);
                if (!type) {
                    SemaValueRuntime *runtime = sema_value_is_runtime(value);
                    if (runtime) {
                        type = runtime->type;
                    }
                }
                if (!type) {
                    sema_module_err(module, segment->slice, "cannot get size of $v", value);
                    return NULL;
                }
                SemaType *usize = sema_module_std_usize(module, segment->slice);
                size_t step_id = sema_expr_output_push_step(ctx.output, hir_expr_step_new_sizeof(
                    sema_type_to_hir(module, type), sema_type_to_hir(module, usize)));
                value = sema_value_new_runtime_expr_step(module->mempool, SEMA_RUNTIME_FINAL, usize, step_id);
                break;
            }
            case AST_PATH_SEGMENT_GENERIC_BUILD: {
                SemaGeneric *generic = NOT_NULL(sema_value_should_be_generic(module, segment->slice, value));
                size_t expected_params = sema_generic_input_count(generic);
                if (expected_params != vec_len(segment->generic->params)) {
                    sema_module_err(module, segment->slice, "expected $l generic parameter, but $l passed",
                        expected_params, vec_len(segment->generic->params));
                    return NULL;
                }
                SemaType **input = vec_new_in(module->mempool, SemaType*);
                vec_resize(input, expected_params);
                for (size_t i = 0; i < expected_params; i++) {
                    input[i] = NOT_NULL(sema_module_type(module, segment->generic->params[i]));
                }
                value = NOT_NULL(sema_generate(generic, input));
                sema_path_ctx_continue_ext(&path_ctx);
                break;
            }
            case AST_PATH_SEGMENT_TYPEOF: {
                SemaValueRuntime *runtime = NOT_NULL(sema_value_should_be_runtime(module, segment->slice, value));
                value = sema_value_new_type(module->mempool, runtime->type);
                break;
            }
        }
        sema_path_ctx_step(&path_ctx);
    }
    return sema_path_ctx_finalize_value(module, &path_ctx, value, ctx.output);
}

SemaValue *sema_module_emit_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx) {
    assert(vec_len(path->segments) > 0);
    SemaValue *value = NOT_NULL(sema_module_path_segment(module, &path->segments[0]));
    return sema_module_emit_expr_path_from(module, value, path, 1, ctx);
}


