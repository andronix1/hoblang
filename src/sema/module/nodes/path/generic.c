#include "generic.h"
#include "ast/api/path.h"
#include "ast/path.h"
#include "sema/module/generic.h"
#include "sema/module/module.h"
#include "sema/module/nodes/generic.h"
#include "sema/module/nodes/path.h"

SemaValue *sema_path_resolve_generic_build(SemaModule *module, SemaValue *value, AstPath *path, size_t idx) {
    sema_path_ignore_before(path, idx);
    SemaGeneric *generic = sema_value_is_generic(value);
    AstPathSegment *segment = &path->segments[idx];
    if (!generic) {
        sema_module_err(module, segment->slice, "cannot build generic from non-generic value");
        return NULL;
    }
    return sema_generic_generate(module, segment->slice, generic, segment->generic);
}
