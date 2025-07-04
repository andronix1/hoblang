#include "type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/mempool.h"
#include "core/vec.h"
#include "sema/module/api/generic.h"
#include "sema/module/generic.h"
#include "sema/module/type/type.h"
#include "sema/module/module.h"
#include <stdio.h>

HirType *sema_type_to_hir(SemaModule* module, SemaType *type) {
    switch (type->kind) {
        case SEMA_TYPE_VOID: return hir_type_new_void(module->mempool);
        case SEMA_TYPE_GEN_PARAM: return hir_type_new_gen(module->mempool, type->gen_param.id);
        case SEMA_TYPE_FLOAT: return hir_type_new_float(module->mempool, sema_type_float_size_to_hir(type->float_size));
        case SEMA_TYPE_BOOL: return hir_type_new_bool(module->mempool);
        case SEMA_TYPE_POINTER: return hir_type_new_pointer(module->mempool, sema_type_to_hir(module, type->pointer_to));
        case SEMA_TYPE_GENERIC: UNREACHABLE;
        case SEMA_TYPE_ARRAY:
            return hir_type_new_array(module->mempool, sema_type_to_hir(module, type->array.of), type->array.length);
        case SEMA_TYPE_INT:
            return hir_type_new_int(module->mempool, sema_type_int_size_to_hir(type->integer.size),
                type->integer.is_signed);
        case SEMA_TYPE_FUNCTION: {
            HirType **args = vec_new_in(module->mempool, HirType*);
            vec_resize(args, vec_len(type->function.args));
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                args[i] = sema_type_to_hir(module, type->function.args[i]);
            }
            return hir_type_new_function(module->mempool, args, sema_type_to_hir(module, type->function.returns));
        }
        case SEMA_TYPE_STRUCTURE: {
            HirTypeStructField *fields = vec_new_in(module->mempool, HirTypeStructField);
            vec_resize(fields, vec_len(type->structure.fields_map));
            for (size_t i = 0; i < vec_len(type->structure.fields_map); i++) {
                keymap_at(type->structure.fields_map, i, field);
                fields[i] = hir_type_struct_field_new(sema_type_to_hir(module, field->value.type));
            }
            return hir_type_new_struct(module->mempool, fields);
        }
        case SEMA_TYPE_RECORD: {
            SemaType *record = type->record.module->types[type->record.id];
            assert(record);
            return sema_type_to_hir(module, record);
        }
        case SEMA_TYPE_GENERATE: return sema_type_to_hir(module, sema_type_generate(type));
        case SEMA_TYPE_ENUM: return sema_type_to_hir(module, type->enumeration.type);
    }
    UNREACHABLE;
}

void sema_type_print(va_list list) {
    SemaType *type = va_arg(list, SemaType*);
    if (type->alias) {
        logs("$S", type->alias->name);
        return;
    }
    switch (type->kind) {
        case SEMA_TYPE_VOID: logs("void"); break;
        case SEMA_TYPE_ARRAY: logs("[$l]$t", type->array.length, type->array.of); break;
        case SEMA_TYPE_FLOAT:
            switch (type->float_size) {
                case SEMA_FLOAT_32: logs("f32"); break;
                case SEMA_FLOAT_64: logs("f64"); break;
            }
            break;
        case SEMA_TYPE_INT:
            logs(type->integer.is_signed ? "i" : "u");
            switch (type->integer.size) {
                case SEMA_INT_8: logs("8"); break;
                case SEMA_INT_16: logs("16"); break;
                case SEMA_INT_32: logs("32"); break;
                case SEMA_INT_64: logs("64"); break;
            }
            break;
        case SEMA_TYPE_BOOL: logs("bool"); break;
        case SEMA_TYPE_FUNCTION:
            logs("fun (");
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                logs(i == 0 ? "$t" : ", $t", type->function.args[i]);
            }
            logs(") -> $t", type->function.returns);
            break;
        case SEMA_TYPE_RECORD:
            logs("$t", type->record.module->types[type->record.id]);
            break;
        case SEMA_TYPE_POINTER:
            logs("*$t", type->pointer_to);
            break;
        case SEMA_TYPE_STRUCTURE:
            logs("structure");
            break;
        case SEMA_TYPE_ENUM:
            logs("enumeration");
            break;
        case SEMA_TYPE_GENERIC: logs("$S", type->generic_name); break;
        case SEMA_TYPE_GEN_PARAM: logs("$S", type->gen_param.name); break;
        case SEMA_TYPE_GENERATE: {
            logs("$S.<", type->generate.generic->name);
            for (size_t i = 0; i < vec_len(type->generate.params); i++) {
                logs(i == 0 ? "$t" : ", $t", type->generate.params[i]);
            }
            logs(">");
            break;
        }
    }
}

SemaType *sema_type_replace(Mempool *mempool, SemaType *source, SemaType **from, SemaType **to) {
    assert(vec_len(from) == vec_len(to));
    for (size_t i = 0; i < vec_len(from); i++) {
        if (from[i] == source) return to[i];
    }
    switch (source->kind) {
        case SEMA_TYPE_VOID: case SEMA_TYPE_INT: case SEMA_TYPE_FLOAT:
        case SEMA_TYPE_BOOL: case SEMA_TYPE_GEN_PARAM: case SEMA_TYPE_ENUM:
            return source;
        case SEMA_TYPE_FUNCTION: {
            SemaType **args = vec_new_in(mempool, SemaType*);
            vec_resize(args, vec_len(source->function.args));
            for (size_t i = 0; i < vec_len(source->function.args); i++) {
                args[i] = sema_type_replace(mempool, source->function.args[i], from, to);
            }
            return sema_type_new_function(mempool, args, sema_type_replace(mempool, source->function.returns, from, to));
        }
        case SEMA_TYPE_POINTER:
            return sema_type_new_pointer(mempool, sema_type_replace(mempool, source->pointer_to, from, to));
        case SEMA_TYPE_STRUCTURE: {
            SemaTypeStructField *fields = keymap_new_in(mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(source->structure.fields_map); i++) {
                keymap_at(source->structure.fields_map, i, field); 
                keymap_insert(fields, field->key, sema_type_struct_field_new(
                    sema_type_replace(mempool, field->value.type, from, to),
                    field->value.module
                ));
            }
            return sema_type_new_structure(mempool, fields);
        }
        case SEMA_TYPE_ARRAY:
            return sema_type_new_array(mempool, source->array.length, sema_type_replace(mempool, source->array.of,
                from, to));
        case SEMA_TYPE_GENERATE: {
            SemaType **new_params = vec_new_in(mempool, SemaType*);
            vec_resize(new_params, vec_len(source->generate.params));
            for (size_t i = 0; i < vec_len(new_params); i++) {
                new_params[i] = sema_type_replace(mempool, source->generate.params[i], from, to);
            }
            return sema_type_new_generate(mempool, source->generate.generic, new_params);
        }
        case SEMA_TYPE_RECORD: case SEMA_TYPE_GENERIC: return source;
    }
    UNREACHABLE;
}

static inline SemaType *sema_type_root_ungenerated(SemaType *type) {
    while (type->kind == SEMA_TYPE_RECORD) {
        type = sema_type_get_record(type);
    }
    return type;
}

SemaType *sema_type_root(SemaType *type) {
    while (true) {
        SemaType *new = sema_type_root_ungenerated(type);
        if (type != new) {
            type = new;
        } else if (type->kind == SEMA_TYPE_GENERATE) {
            type = sema_type_generate(type);
        } else {
            break;
        }
    }
    return type;
}

bool sema_type_can_be_downcasted(SemaType *type, SemaType *to) {
    if (type == to || (type->alias && type->alias == to->alias)) {
        return true;
    }
    if (type->kind != to->kind) {
        if (type->kind == SEMA_TYPE_RECORD) {
            return sema_type_can_be_downcasted(sema_type_get_record(type), to);
        } else if (type->kind == SEMA_TYPE_GENERATE) {
            return sema_type_can_be_downcasted(sema_generic_type_generate(type->generate.generic, type->generate.params),
                to);
        }
        return false;
    }
    switch (type->kind) {
        case SEMA_TYPE_GEN_PARAM: case SEMA_TYPE_GENERIC: case SEMA_TYPE_STRUCTURE: case SEMA_TYPE_ENUM: return false;
        case SEMA_TYPE_RECORD:
            return (type->record.module == to->record.module && type->record.id == to->record.id) || 
                sema_type_can_be_downcasted(sema_type_get_record(type), to);
        case SEMA_TYPE_VOID: case SEMA_TYPE_BOOL: return true;
        case SEMA_TYPE_INT:
            return type->integer.size == to->integer.size && type->integer.is_signed == to->integer.is_signed;
        case SEMA_TYPE_FUNCTION:
            if (vec_len(type->function.args) != vec_len(to->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(type->function.args); i++) {
                if (!sema_type_can_be_downcasted(type->function.args[i], to->function.args[i])) {
                    return false;
                }
            }
            return sema_type_can_be_downcasted(type->function.returns, to->function.returns);
        case SEMA_TYPE_POINTER: return sema_type_can_be_downcasted(type->pointer_to, to->pointer_to);
        case SEMA_TYPE_ARRAY:
            return sema_type_can_be_downcasted(type->array.of, to->array.of) && type->array.length == to->array.length;
        case SEMA_TYPE_FLOAT: return type->float_size == to->float_size;
        case SEMA_TYPE_GENERATE: {
            if (type->generate.generic != to->generate.generic || vec_len(type->generate.params) != vec_len(to->generate.params)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(type->generate.params); i++) {
                if (!sema_type_can_be_downcasted(type->generate.params[i], to->generate.params[i])) {
                    return false;
                }
            }
            return true;
        }
    }
    UNREACHABLE;
}

bool sema_type_can_be_casted(SemaType *type, SemaType *to) {
    return sema_type_can_be_downcasted(type, to) || sema_type_can_be_downcasted(to, type);
}
