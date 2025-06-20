#pragma once

#include "ast/api/generic.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/module.h"
#include "sema/module/generic.h"
#include "sema/module/scope.h"

typedef struct {
    bool is_global;
    SemaScopeStack *ss;
} SemaGenericCtx;

SemaGeneric *sema_module_generic(SemaModule *module, SemaGenericKind kind, AstGeneric *generic);
SemaGenericCtx sema_module_generic_ctx_setup(SemaModule *module, AstGeneric *generic, SemaGeneric *source);
void sema_module_generic_ctx_clean(SemaModule *module, SemaGenericCtx ctx);
