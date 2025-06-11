#pragma once

#include "ast/api/expr.h"
#include "core/mempool.h"
#include "ir/stmt/expr.h"
#include "sema/module/api/value.h"

typedef struct {
    IrExprStep *steps;
} SemaExprOutput;

typedef struct {
    SemaExprOutput *output;
    SemaType *expectation;
} SemaExprCtx;

static inline SemaExprOutput sema_expr_output_new_with(IrExprStep *steps) {
    SemaExprOutput output = { .steps = steps };
    return output;
}

static inline SemaExprOutput sema_expr_output_new(Mempool *mempool) {
    return sema_expr_output_new_with(vec_new_in(mempool, IrExprStep));
}

static inline SemaExprCtx sema_expr_ctx_new(SemaExprOutput *output, SemaType *expectation) {
    SemaExprCtx ctx = {
        .expectation = expectation,
        .output = output,
    };
    return ctx;
}

size_t sema_module_expr_emit_runtime(SemaValueRuntime *runtime, SemaExprOutput *output);
size_t sema_expr_output_push_step(SemaExprOutput *output, IrExprStep step);
size_t sema_expr_output_last_id(SemaExprOutput *output);
IrExpr sema_expr_output_collect(SemaExprOutput *output);

SemaValue *sema_module_emit_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx);
SemaValueRuntime *sema_module_emit_runtime_expr(SemaModule *module, AstExpr *expr, SemaExprCtx ctx);
