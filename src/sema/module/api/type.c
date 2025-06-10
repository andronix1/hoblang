#include "type.h"
#include "core/assert.h"
#include "core/log.h"
#include "core/vec.h"
#include "sema/module/type.h"
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
    }
}

bool sema_type_eq(const SemaType *a, const SemaType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case SEMA_TYPE_VOID:
        case SEMA_TYPE_BOOL:
            return true;
        case SEMA_TYPE_RECORD: return a->type_id == b->type_id;
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
