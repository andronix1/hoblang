#include "if.h"
#include "ast/stmt.h"
#include "core/null.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/body.h"
#include "sema/module/nodes/expr.h"
#include "sema/module/type.h"

static bool sema_module_analyze_cond_block(SemaModule *module, AstCondBlock *cond_block) {
    SemaType *boolean = sema_type_new_primitive_bool(module->mempool);
    SemaType *type = sema_value_is_runtime(NOT_NULL(sema_module_analyze_expr(module, cond_block->cond,
        sema_expr_ctx_new(boolean))));
    if (!type) {
        sema_module_err(module, cond_block->cond->slice, "condition cannot be non-runtime value", type);
    } else if (!sema_type_eq(type, boolean)) {
        sema_module_err(module, cond_block->cond->slice, "expected boolean got $t", type);
    }
    return sema_module_analyze_body(module, cond_block->body);
}

bool sema_module_analyze_if(SemaModule *module, AstIf *if_else) {
    assert(vec_len(if_else->conds) > 0);
    bool breaks = false;
    for (size_t i = 0; i < vec_len(if_else->conds); i++) {
        if (sema_module_analyze_cond_block(module, &if_else->conds[i])) {
            breaks = true;
        }
    }
    if (if_else->else_body) {
        if (sema_module_analyze_body(module, if_else->else_body)) {
            breaks = true;
        }
    } else {
        breaks = true;
    }
    return breaks;
}
