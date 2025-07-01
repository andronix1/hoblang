#pragma once

#include "core/slice.h"
#include "sema/module/type/ext.h"

typedef struct SemaTypeAlias {
    Slice name;
    SemaExtDecl *decls_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, Slice name);
void sema_type_alias_add_extension(SemaModule *module, SemaTypeAlias *alias, Slice name, SemaExtDecl decl);
