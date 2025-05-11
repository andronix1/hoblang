#include "path.h"
#include "ast/path.h"
#include "core/assert.h"
#include "sema/module/module.h"

SemaValue *sema_module_resolve_path(SemaModule *module, AstPath *path) {
    assert(vec_len(path->segments) > 0);
    if (vec_len(path->segments) != 1) {
        TODO;
    }
    assert(path->segments->kind == AST_PATH_SEGMENT_IDENT);
    const SemaDecl *decl = sema_module_resolve_decl(module, path->segments->ident);
    if (!decl) {
        sema_module_err(module, path->segments->ident, "`$S` is undefined", path->segments->ident);
        return NULL;
    }
    return decl->handle->value;
}
