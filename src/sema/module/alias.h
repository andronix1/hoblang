#pragma once

#include "core/mempool.h"
#include "sema/module/module.h"

typedef enum {
    SEMA_TYPE_ALIAS_EXT_FUN,
} SemaTypeAliasExtKind;

typedef struct {
    SemaTypeAliasExtKind kind;

    union {
        SemaDecl decl;
    };
} SemaTypeAliasExt;

typedef struct SemaTypeAlias {
    SemaTypeAliasExt *ext_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool);
SemaDeclHandle *sema_type_alias_try_resolve(SemaTypeAlias *alias, Slice name);

static inline SemaTypeAliasExt sema_type_alias_ext_new_fun(SemaDecl decl) {
    SemaTypeAliasExt ext = {
        .kind = SEMA_TYPE_ALIAS_EXT_FUN,
        .decl = decl,
    };
    return ext;
}
