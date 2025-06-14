#pragma once

#include "ir/api/decl.h"
#include "sema/module/api/type.h"
#include <stdbool.h>

typedef struct SemaStringModule {
    bool loaded;
    SemaType *usize;
    struct {
        SemaType *type;
        IrDeclId new;
    } string;
} SemaInternalModuleInfo;

static inline SemaInternalModuleInfo sema_internal_module_new() {
    SemaInternalModuleInfo str = { .loaded = false };
    return str;
}

IrDeclId *sema_module_internal_string_new(SemaModule *module, Slice at);

