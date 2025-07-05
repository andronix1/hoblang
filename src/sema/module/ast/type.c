#include "type.h"
#include "ast/expr.h"
#include "ast/type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/mempool.h"
#include "core/null.h"
#include "core/vec.h"
#include "sema/module/api/type.h"
#include "sema/module/api/value.h"
#include "sema/module/ast/path.h"
#include "sema/module/const.h"
#include "sema/module/exprs/expr.h"
#include "sema/module/module.h"
#include "sema/module/type/type.h"

static inline size_t get_int_bits(SemaType *type) {
    type = sema_type_root(type);
    if (type->kind != SEMA_TYPE_INT) {
        return 0;
    }
    switch (type->integer.size) {
        case SEMA_INT_8: return 8 - type->integer.is_signed;
        case SEMA_INT_16: return 16 - type->integer.is_signed;
        case SEMA_INT_32: return 32 - type->integer.is_signed;
        case SEMA_INT_64: return 64 - type->integer.is_signed;
    }
    UNREACHABLE;
}

static inline size_t get_max_value(size_t bits) {
    size_t res = 0;
    for (size_t i = 0; i < bits; i++) {
        res = (res << 1) | 1;
    }
    return res;
}

static inline SemaType *detect_enum_type(SemaModule *module, AstType *explicit_type, size_t max_variants) {
    if (explicit_type) {
        SemaType *type = NOT_NULL(sema_module_type(module, explicit_type));
        size_t max_value = get_max_value(get_int_bits(type));
        if (!max_value) {
            sema_module_err(module, explicit_type->slice, "type is not an integer");
            return NULL;
        }
        if (max_value < max_variants) {
            sema_module_err(module, explicit_type->slice, "variants count more than max value can be reached with specified type ($l > $l)", max_variants, max_value);
            return NULL;
        }
    }
    if (max_variants < 0xFF) {
        return sema_type_new_int(module->mempool, SEMA_INT_8, false);
    } else if (max_variants < 0xFFFF) {
        return sema_type_new_int(module->mempool, SEMA_INT_16, false);
    } else if (max_variants < 0xFFFFFFFF) {
        return sema_type_new_int(module->mempool, SEMA_INT_32, false);
    } else {
        // I don't know, who can reach this limit btw
        //
        // Pointers are too small to enumerate RAM wanted
        // for store such big number of variants, but... why not?)
        return sema_type_new_int(module->mempool, SEMA_INT_64, false);
    }
}

SemaType *sema_module_type(SemaModule *module, AstType *type) {
    switch (type->kind) {
        case AST_TYPE_PATH: {
            return NOT_NULL(sema_value_should_be_type(module, type->slice,
                NOT_NULL(sema_module_path(module, type->path))));
        }
        case AST_TYPE_ENUM: {
            size_t max_variant = vec_len(type->enumeration.variants_map);
            if (!type->enumeration.explicit_type) {
                for (size_t i = 0; i < vec_len(type->enumeration.variants_map); i++) {
                    keymap_at(type->enumeration.variants_map, i, var);
                    if (var->value.expr) {
                        max_variant = 0xFFFFFFFF;
                        break;
                    }
                }
            }
            SemaType *enum_type = NOT_NULL(detect_enum_type(module, type->enumeration.explicit_type, max_variant));
            SemaEnumVariant *variants = keymap_new_in(module->mempool, SemaEnumVariant);
            size_t inc = -1;
            for (size_t i = 0; i < vec_len(type->enumeration.variants_map); i++) {
                inc++;
                keymap_at(type->enumeration.variants_map, i, var);
                if (var->value.expr) {
                    SemaValueRuntime *runtime = sema_module_emit_runtime_expr_full(module, var->value.expr,
                        sema_expr_ctx_new(NULL, enum_type));
                    if (!runtime) {
                        continue;
                    }
                    if (!sema_type_can_be_downcasted(runtime->type, enum_type)) {
                        sema_module_err(module, var->value.expr->slice,
                            "expression of type $t cannot be downcasted to type $t", runtime->type, enum_type);
                        continue;
                    }
                    SemaConst *constant = sema_value_runtime_should_be_constant(module, var->value.expr->slice, runtime);
                    if (!constant) {
                        continue;
                    }
                    assert(constant->kind == SEMA_CONST_INT);
                    inc = constant->integer;
                }
                keymap_insert(variants, var->key, sema_enum_variant_new(inc));
            }
            return sema_type_new_enum(module->mempool, enum_type, variants);
        }
        case AST_TYPE_POINTER:
            return sema_type_new_pointer(module->mempool, NOT_NULL(sema_module_type(module, type->pointer_to)));
        case AST_TYPE_FUNCTION: {
            SemaType **args = vec_new_in(module->mempool, SemaType*);
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                vec_push(args, NOT_NULL(sema_module_type(module, type->function.args[i])));
            }
            SemaType *returns = type->function.returns ?
                NOT_NULL(sema_module_type(module, type->function.returns)) :
                sema_type_new_void(module->mempool);
            return sema_type_new_function(module->mempool, args, returns);
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
            return sema_type_new_array(module->mempool, constant->integer, of);
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
            return sema_type_new_structure(module->mempool, fields);
        }
    }
    UNREACHABLE;
}

SemaType *sema_module_opt_type(SemaModule *module, AstType *type) {
    return type ? NOT_NULL(sema_module_type(module, type)) : sema_type_new_void(module->mempool);
}
