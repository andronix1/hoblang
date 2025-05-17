#include "ident.h"
#include "ast/path.h"
#include "core/keymap.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/path_ext.h"
#include "sema/module/type.h"
#include "sema/module/value.h"
#include "sema/module/alias.h"
#include <stdio.h>

SemaValue *sema_path_resolve_ident(SemaModule *module, SemaValue *value, AstPath *path, size_t idx) {
    SemaType *type = sema_value_is_runtime(value);
    AstPathSegment *segment = &path->segments[idx];
    if (!type) {
        sema_module_err(module, segment->ident, "cannot get `$S` from non-runtime value", segment->ident);
        return NULL;
    }
    if (type->kind == SEMA_TYPE_STRUCT) {
        size_t idx = keymap_get_idx(type->structure.fields_map, segment->ident);
        if (idx != -1) {
            keymap_at(type->structure.fields_map, idx, field);
            segment->sema.kind = SEMA_PATH_SEGMENT_STRUCT_FIELD;
            segment->sema.struct_field_idx = idx;
            return sema_value_new_nested(module->mempool, value, field->value.type);
        }
    }
    if (type->alias) {
        SemaTypeAliasExt *type_ext = sema_type_alias_try_resolve(type->alias, segment->ident);
        if (type_ext) {
            SemaDeclHandle *handle = type_ext->decl.handle;
            SemaDeclHandle *ext = sema_decl_handle_new(module->mempool, handle->value);
            if (type_ext->fun.by_ref) {
                if (!sema_value_is_var(value)) {
                    sema_module_err(module, segment->slice, "extension function is taking reference");
                }
                segment->sema.kind = SEMA_PATH_SEGMENT_EXT_REF;
                segment->sema.ext.decl = handle;
                segment->sema.ext.handle = ext;
            } else {
                segment->sema.kind = SEMA_PATH_SEGMENT_EXT_DIRECT;
                segment->sema.ext.decl = handle;
                segment->sema.ext.handle = ext;
            }
            return sema_value_new_ext(module->mempool, handle->value, ext);
        }
    }
    sema_module_err(module, segment->ident, "there is not `$S` in $t", segment->ident, type);
    return NULL;
}
