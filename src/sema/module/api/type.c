#include "type.h"
#include "core/assert.h"
#include "core/vec.h"
#include "sema/module/type.h"

IrTypeId sema_type_ir_id(SemaType *type) {
    if (type->aliases) {
        return (*vec_top(type->aliases))->id;
    }
    return type->ir_id;
}

bool sema_type_eq(const SemaType *a, const SemaType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case SEMA_TYPE_VOID: return true;
        case SEMA_TYPE_BOOL: return true;
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
    }
    UNREACHABLE;
}
