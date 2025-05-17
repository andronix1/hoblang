#include "path.h"
#include "ast/path.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/path/generic.h"
#include "sema/module/nodes/path/ident.h"
#include "sema/module/nodes/path_ext.h"
#include "sema/module/type.h"
#include "sema/module/value.h"

void sema_path_ignore_before(AstPath *path, size_t before) {
    for (size_t i = 0; i < before; i++) {
        path->segments[i].sema.kind = SEMA_PATH_SEGMENT_IGNORE;
    }
}

static SemaValue *_sema_module_resolve_path_in(SemaModule *module, SemaValue *value, AstPath *path, size_t offset) {
    for (size_t i = offset; i < vec_len(path->segments); i++) {
        AstPathSegment *segment = &path->segments[i];
        segment->sema.from_value = value;
        switch (segment->kind) {
            case AST_PATH_SEGMENT_IDENT: {
                value = NOT_NULL(sema_path_resolve_ident(module, value, path, i));
                break;
            }
            case AST_PATH_SEGMENT_DEREF: {
                SemaType *type = sema_value_is_runtime(value);
                if (!type) {
                    sema_module_err(module, segment->slice, "cannot deref non-runtime value");
                    return NULL;
                }
                if (type->kind != SEMA_TYPE_POINTER) {
                    sema_module_err(module, segment->slice, "ony pointers can be dereferenced, not $t", type);
                    return NULL;
                }
                segment->sema.kind = SEMA_PATH_SEGMENT_DEREF;
                value = NOT_NULL(sema_value_new_var(module->mempool, type->pointer_to));
                break;
            }
            case AST_PATH_SEGMENT_GENERIC_BUILD: {
                value = NOT_NULL(sema_path_resolve_generic_build(module, value, path, i));
                break;
            }
        }
    }
    return value;
}

SemaValue *sema_module_resolve_path(SemaModule *module, AstPath *path) {
    assert(vec_len(path->segments) > 0);
    assert(path->segments->kind == AST_PATH_SEGMENT_IDENT);
    SemaDeclHandle *decl = sema_module_resolve_decl(module, path->segments->ident);
    if (!decl) {
        sema_module_err(module, path->segments->ident, "`$S` is undefined", path->segments->ident);
        return NULL;
    }
    path->segments->sema.kind = SEMA_PATH_SEGMENT_DECL;
    path->segments->sema.decl = decl;
    return _sema_module_resolve_path_in(module, decl->value, path, 1);
}
