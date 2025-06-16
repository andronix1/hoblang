#pragma once

#include "ast/expr.h"
#include "sema/module/api/value.h"
#include "sema/module/exprs/expr.h"

bool sema_type_to_ir_number_info(SemaType *type, IrNumberInfo *output);
SemaValue *sema_module_append_expr_binop(SemaModule *module, SemaType *type, size_t ls, size_t rs, AstBinopKind *kind, SemaExprOutput *output);
SemaValue *sema_module_emit_expr_binop(SemaModule *module, AstBinop *binop, SemaExprCtx ctx);


