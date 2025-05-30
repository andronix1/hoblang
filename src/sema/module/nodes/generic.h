#pragma once

#include "ast/api/generic.h"
#include "sema/module/api.h"
#include "sema/module/api/generic.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"

typedef struct SemaGenericScopeHandle {
    bool is_global;
    SemaScopeStack ss;
    SemaScopeStack *old;
    SemaGeneric *generic;
} SemaGenericScopeHandle;

SemaValue *sema_value_generate(SemaModule *module, Slice where, SemaGeneric *generic, SemaType **params);
void sema_module_generic_setup(SemaModule *module, AstGeneric *generic, SemaGenericScopeHandle *handle);
void sema_module_generic_clean(SemaModule *module, SemaType *type, SemaGenericScopeHandle *handle);
SemaValue *sema_generic_generate(SemaModule *module, Slice where, SemaGeneric *generic, AstGenericBuilder *builder);
