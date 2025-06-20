#include "type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/vec.h"
#include "sema/module/decl.h"
#include "sema/module/generic.h"
#include "sema/module/type.h"
#include "sema/module/module.h"
#include <stdio.h>

HirTypeId sema_type_hir_id(SemaType *type) {
    if (type->aliases) {
        return (*vec_top(type->aliases))->id;
    }
    return type->hir_id;
}

void sema_type_print(va_list list) {
    SemaType *type = va_arg(list, SemaType*);
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
            logs("<alias>", type->pointer_to);
            break;
        case SEMA_TYPE_POINTER:
            logs("*$t", type->pointer_to);
            break;
        case SEMA_TYPE_STRUCTURE:
            logs("structure", type->pointer_to);
            break;
        case SEMA_TYPE_GENERIC:
            logs("generic");
            break;
        case SEMA_TYPE_GENERATE:
            logs("generator");
            break;
    }
}

static SemaType *sema_type_generate(SemaModule *module, SemaType *source, SemaType **params, SemaType **input) {
    for (size_t i = 0; i < vec_len(params); i++) {
        if (params[i] == source) return input[i];
    }
    switch (source->kind) {
        case SEMA_TYPE_VOID:
        case SEMA_TYPE_INT:
        case SEMA_TYPE_FLOAT:
        case SEMA_TYPE_BOOL:
            return source;
        case SEMA_TYPE_FUNCTION: {
            SemaType **args = vec_new_in(module->mempool, SemaType*);
            vec_resize(args, vec_len(source->function.args));
            for (size_t i = 0; i < vec_len(source->function.args); i++) {
                args[i] = sema_type_generate(module, source->function.args[i], params, input);
            }
            return sema_type_new_function(module, args,
                sema_type_generate(module, source->function.returns, params, input));
        }
        case SEMA_TYPE_POINTER:
            return sema_type_new_pointer(module, sema_type_generate(module, source->pointer_to, params, input));
        case SEMA_TYPE_STRUCTURE: {
            SemaTypeStructField *fields = keymap_new_in(module->mempool, SemaTypeStructField);
            for (size_t i = 0; i < vec_len(source->structure.fields_map); i++) {
                keymap_at(source->structure.fields_map, i, field); 
                keymap_insert(fields, field->key, sema_type_struct_field_new(
                    sema_type_generate(module, field->value.type, params, input),
                    field->value.module
                ));
            }
            return sema_type_new_structure(module, fields);
        }
        case SEMA_TYPE_ARRAY:
            return sema_type_new_array(module, source->array.length, sema_type_generate(module, source->array.of,
                params, input));
        case SEMA_TYPE_RECORD: return sema_type_generate(module, sema_type_resolve(source), params, input);
        case SEMA_TYPE_GENERATE: case SEMA_TYPE_GENERIC: return source;
    }
    UNREACHABLE;
}

SemaType *sema_type_resolve(SemaType *type) {
    while (true) {
        if (type->kind == SEMA_TYPE_RECORD) {
            type = type->record.module->types[type->record.id].type;
        } else if (type->kind == SEMA_TYPE_GENERATE) {
            SemaGeneric *generic = type->generate.generic;
            assert(generic->kind == SEMA_GENERIC_TYPE);
            type = sema_type_generate(generic->module, type->generate.generic->type,
                    type->generate.generic->params, type->generate.params);
        } else {
            break;
        }
    }
    return type;
}

bool sema_type_eq(SemaType *a, SemaType *b) {
    a = sema_type_resolve(a);
    b = sema_type_resolve(b);
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case SEMA_TYPE_VOID:
        case SEMA_TYPE_BOOL:
            return true;
        case SEMA_TYPE_RECORD: UNREACHABLE;
        case SEMA_TYPE_GENERIC: case SEMA_TYPE_STRUCTURE: return a == b;
        case SEMA_TYPE_INT:
            return
                a->integer.size == b->integer.size &&
                a->integer.is_signed == b->integer.is_signed;
        case SEMA_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->function.args); i++) {
                if (!sema_type_eq(a->function.args[i], b->function.args[i])) {
                    return false;
                }
            }
            return sema_type_eq(a->function.returns, b->function.returns);
        case SEMA_TYPE_POINTER:
            return sema_type_eq(a->pointer_to, b->pointer_to);
        case SEMA_TYPE_ARRAY:
            return sema_type_eq(a->array.of, b->array.of) && a->array.length == b->array.length;
        case SEMA_TYPE_FLOAT:
            return a->float_size == b->float_size;
        case SEMA_TYPE_GENERATE:
            if (vec_len(a->generate.params) != vec_len(b->generate.params)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->generate.params); i++) {
                if (!sema_type_eq(a->generate.params[i], b->generate.params[i])) {
                    return false;
                }
            }
            return a->generate.generic == b->generate.generic;
    }
    UNREACHABLE;
}

SemaDecl *sema_type_search_ext(SemaModule *module, SemaType *type, Slice name) {
    if (!type->aliases) {
        return NULL;
    }
    for (ssize_t i = vec_len(type->aliases) - 1; i >= 0; i--) {
        SemaDecl **decl = keymap_get(type->aliases[i]->decls_map, name);
        if (decl) {
            if ((*decl)->module != NULL && (*decl)->module != module) {
                sema_module_err(module, name, "`$S` is private", name);
                return NULL;
            }
            return *decl;
        }
    }
    if (type->kind == SEMA_TYPE_RECORD) {
        return sema_type_search_ext(module, type->record.module->types[type->record.id].type, name);
    }
    return NULL;
}
