#pragma once

#include "core/mempool.h"
#include "sema/module/module.h"

typedef enum {
    SEMA_TYPE_ALIAS_EXT_FUN,
} SemaTypeAliasExtKind;

typedef struct {
    SemaTypeAliasExtKind kind;

    SemaDecl decl;

    union {
        struct {
            bool by_ref;
        } fun;
    };
} SemaTypeAliasExt;

typedef struct SemaTypeAlias {
    SemaTypeAliasExt *ext_map;
} SemaTypeAlias;

SemaTypeAlias *sema_type_alias_new(Mempool *mempool);
SemaTypeAliasExt *sema_type_alias_try_resolve(SemaTypeAlias *alias, Slice name);

static inline SemaTypeAliasExt sema_type_alias_ext_new_fun(SemaDecl decl, bool by_ref) {
    SemaTypeAliasExt ext = {
        .kind = SEMA_TYPE_ALIAS_EXT_FUN,
        .decl = decl,
        .fun = {
            .by_ref = by_ref
        }
    };
    return ext;
}
