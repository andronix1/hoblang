#include "path.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/value.h"
#include <stdio.h>

static SemaValue *sema_module_analyze_expr_path_ident(SemaModule *module, SemaValue *value, Slice ident, SemaExprOutput *output) {
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (runtime) {
        size_t of = sema_module_expr_get_runtime(runtime, output);
        SemaDecl *decl = sema_type_search_ext(runtime->type, ident);
        if (decl) {
            SemaValueRuntime *decl_runtime = sema_value_is_runtime(decl->value);
            if (decl_runtime) {
                size_t step_id = sema_module_expr_get_runtime(decl_runtime, output);
                return sema_value_new_runtime_ext_expr_step(module->mempool, decl_runtime->kind,
                    decl_runtime->type, step_id, of);
            }
        }
    }
    sema_module_err(module, ident, "cannot find `$S` in $v", ident, value);
    return NULL;
}

SemaValue *sema_module_analyze_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx) {
    assert(vec_len(path->segments) > 0);
    SemaValue *value = NOT_NULL(sema_module_path_segment(module, &path->segments[0]));
    for (size_t i = 1; i < vec_len(path->segments); i++) {
        AstPathSegment *segment = &path->segments[i];
        switch (segment->kind) {
            case AST_PATH_SEGMENT_IDENT:
                value = NOT_NULL(sema_module_analyze_expr_path_ident(module, value, segment->ident, ctx.output));
                break;
            case AST_PATH_SEGMENT_GENERIC_BUILD:
            case AST_PATH_SEGMENT_DEREF:
                TODO;
        }
    }
    return value;
}


