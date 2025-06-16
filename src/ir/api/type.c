#include "type.h"
#include "core/assert.h"
#include "ir/type/type.h"

bool ir_type_eq(const IrType *a, const IrType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case IR_TYPE_VOID:
        case IR_TYPE_BOOL:
            return true;
        case IR_TYPE_POINTER:
            return a->pointer_to == b->pointer_to;
        case IR_TYPE_INT:
            return
                a->integer.is_signed == b->integer.is_signed && 
                a->integer.size == b->integer.size;
        case IR_TYPE_FLOAT:
            return a->float_size == b->float_size;
        case IR_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->function.args); i++) {
                if (a->function.args[i] != b->function.args[i]) {
                    return false;
                }
            }
            return a->function.returns == b->function.returns;
        case IR_TYPE_STRUCT:
            if (vec_len(a->structure.fields) != vec_len(b->structure.fields)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields); i++) {
                if (a->structure.fields[i] != b->structure.fields[i]) {
                    return false;
                }
            }
            return true;
        case IR_TYPE_ARRAY:
            return a->array.length == b->array.length && a->array.of == b->array.of;
    }
    UNREACHABLE;
}
