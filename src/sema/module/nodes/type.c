#include "type.h"
#include "ast/path.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/value.h"
#include "sema/module/module.h"
#include "sema/module/nodes/path.h"
#include "sema/module/type.h"

SemaType *sema_module_analyze_type(SemaModule *module, AstType *type) {
    switch (type->kind) {
        case AST_TYPE_STRUCT: {
            SemaTypeStructField *fields_map = keymap_new_in(module->mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                SemaType *type = sema_module_analyze_type(module, field->value.type);
                if (!type) {
                    continue;
                }
                keymap_insert(fields_map, field->key, sema_type_struct_field_new(field->value.is_local, type));
            }
            return sema_type_new_struct(module->mempool, fields_map);
        }
        case AST_TYPE_PATH: {
            SemaValue *value = NOT_NULL(sema_module_resolve_path(module, type->path));
            SemaType *sema_type = sema_value_is_type(value);
            if (!sema_type) {
                sema_module_err(module, ast_path_slice(type->path), "expected type");
            }
            return sema_type;
        }
    }
    UNREACHABLE;
}
