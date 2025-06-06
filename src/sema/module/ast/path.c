#include "path.h"
#include "ast/path.h"
#include "core/null.h"
#include "sema/module/api/value.h"
#include "sema/module/decl.h"
#include "sema/module/module.h"
#include <assert.h>

SemaValue *sema_module_path_segment(SemaModule *module, AstPathSegment *segment) {
    assert(segment->kind == AST_PATH_SEGMENT_IDENT);  
    SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl(module, segment->ident));
    return decl->value;
}

SemaValue *sema_module_path(SemaModule *module, AstPath *path) {
    assert(vec_len(path->segments) == 1);
    SemaDecl *decl = NOT_NULL(sema_module_resolve_req_decl(module, path->segments->ident));
    return decl->value;
}
