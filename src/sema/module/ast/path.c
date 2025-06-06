#include "path.h"
#include "ast/path.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"

SemaValue *sema_module_path(SemaModule *module, AstPath *path) {
    assert(vec_len(path->segments) != 1 || path->segments->kind == AST_PATH_SEGMENT_IDENT);  
    SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl(module, path->segments->ident));
    return decl->value;
}
