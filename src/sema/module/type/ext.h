#pragma once

#include "sema/module/api/module.h"
#include "sema/module/api/value.h"

typedef struct {
    SemaValue *function;
    SemaModule *module;
    bool by_ref;
} SemaExtDecl;

static inline SemaExtDecl sema_alias_decl_new(SemaValue *function, SemaModule *module, bool by_ref) {
    SemaExtDecl decl = {
        .function = function,
        .module = module,
        .by_ref = by_ref,
    };
    return decl;
}

