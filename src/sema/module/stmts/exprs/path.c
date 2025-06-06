#include "path.h"
#include "ast/path.h"
#include "core/assert.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/module.h"
#include "sema/module/stmts/expr.h"
#include "sema/module/value.h"

SemaValue *sema_module_analyze_expr_path(SemaModule *module, AstPath *path, SemaExprCtx ctx) {
    assert(vec_len(path->segments) > 0);
    SemaValue *value = NOT_NULL(sema_module_path_segment(module, &path->segments[0]));
    SemaValueRuntime *runtime = sema_value_is_runtime(value);
    if (runtime) {
        value = sema_value_new_runtime_expr_step(module->mempool, runtime->type,
            sema_module_expr_get_runtime(runtime, ctx.output));
    }
    for (size_t i = 1; i < vec_len(path->segments); i++) {
        AstPathSegment *segment = &path->segments[i];
        switch (segment->kind) {
            case AST_PATH_SEGMENT_IDENT:
            case AST_PATH_SEGMENT_GENERIC_BUILD:
            case AST_PATH_SEGMENT_DEREF:
                TODO;
        }
    }
    return value;
}


