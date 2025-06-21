#include "type.h"
#include "ast/expr.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/const.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"
#include "sema/module/type.h"

SemaType *sema_module_type(SemaModule *module, AstType *type) {
    switch (type->kind) {
        case AST_TYPE_PATH: {
            return NOT_NULL(sema_value_should_be_type(module, type->slice,
                NOT_NULL(sema_module_path(module, type->path))));
        }
        case AST_TYPE_POINTER:
            return sema_type_new_pointer(module, NOT_NULL(sema_module_type(module, type->pointer_to)));
        case AST_TYPE_FUNCTION: {
            SemaType **args = vec_new_in(module->mempool, SemaType*);
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                vec_push(args, NOT_NULL(sema_module_type(module, type->function.args[i])));
            }
            SemaType *returns = type->function.returns ?
                NOT_NULL(sema_module_type(module, type->function.returns)) :
                sema_type_new_void(module);
            return sema_type_new_function(module, args, returns);
        }
        case AST_TYPE_ARRAY: {
            SemaType *of = NOT_NULL(sema_module_type(module, type->array.type));
            SemaType *usize = sema_module_std_usize(module, type->array.type->slice);
            SemaConst *constant = NOT_NULL(sema_value_runtime_should_be_constant(module, type->slice,
                sema_module_emit_runtime_expr_full(module, type->array.length, sema_expr_ctx_new(NULL, usize))));
            if (!sema_type_can_be_casted(constant->type, usize)) {
                sema_module_err(module, type->array.length->slice, "array size must be usize");
                return NULL;
            }
            assert(constant->kind == SEMA_CONST_INT);
            return sema_type_new_array(module, constant->integer, of);
        }
        case AST_TYPE_STRUCT: {
            SemaTypeStructField *fields = keymap_new_in(module->mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                keymap_insert(fields, field->key, sema_type_struct_field_new(
                    NOT_NULL(sema_module_type(module, field->value.type)),
                    field->value.is_public ? NULL : module
                ));
            }
            return sema_type_new_structure(module, fields);
        }
    }
    UNREACHABLE;
}

SemaType *sema_module_opt_type(SemaModule *module, AstType *type) {
    return type ? NOT_NULL(sema_module_type(module, type)) : sema_type_new_void(module);
}
