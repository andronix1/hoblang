#include "alias.h"
#include "core/mempool.h"
#include "sema/module/module.h"

SemaTypeAlias *sema_type_alias_new(Mempool *mempool, Slice name)
    MEMPOOL_CONSTRUCT(SemaTypeAlias,
        out->name = name;
        out->decls_map = keymap_new_in(mempool, SemaExtDecl);
    )
    
void sema_type_alias_add_extension(SemaModule *module, SemaTypeAlias *alias, Slice name, SemaExtDecl decl) {
    if (keymap_insert(alias->decls_map, name, decl)) {
        sema_module_err(module, name, "extension `$S` already defined", name);
        return;
    }
}
