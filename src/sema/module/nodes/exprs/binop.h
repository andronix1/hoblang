#pragma once

#include "ast/api/expr.h"
#include "sema/module/api.h"
#include "sema/module/api/value.h"

SemaValue *sema_module_analyze_binop(SemaModule *module, SemaType *a, SemaType *b, AstBinopKind *binop);
