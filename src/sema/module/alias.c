#include "alias.h"
#include "core/mempool.h"
#include "core/null.h"

SemaTypeAliasExt *sema_type_alias_try_resolve(SemaTypeAlias *alias, Slice name) {
    return NOT_NULL(keymap_get(alias->ext_map, name));
}

SemaTypeAlias *sema_type_alias_new(Mempool *mempool)
    MEMPOOL_CONSTRUCT(SemaTypeAlias, out->ext_map = keymap_new_in(mempool, SemaTypeAliasExt))
