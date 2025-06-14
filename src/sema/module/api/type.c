#include "type.h"
#include "core/assert.h"
#include "core/keymap.h"
#include "core/log.h"
#include "core/vec.h"
#include "sema/module/decl.h"
#include "sema/module/type.h"
#include "sema/module/module.h"
#include <stdio.h>

IrTypeId sema_type_ir_id(SemaType *type) {
    if (type->aliases) {
        return (*vec_top(type->aliases))->id;
    }
    return type->ir_id;
}

void sema_type_print(va_list list) {
    SemaType *type = va_arg(list, SemaType*);
    switch (type->kind) {
        case SEMA_TYPE_VOID: logs("void"); break;
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
    }
}

SemaType *sema_type_resolve(SemaType *type) {
    while (type->kind == SEMA_TYPE_RECORD) {
        type = type->record.module->types[type->record.id].type;
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
        case SEMA_TYPE_STRUCTURE: return a == b;
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
    return NULL;
}
