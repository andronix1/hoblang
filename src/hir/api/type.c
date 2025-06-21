#include "type.h"
#include "core/assert.h"
#include "core/vec.h"

bool hir_type_eq(const HirType *a, const HirType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case HIR_TYPE_VOID:
        case HIR_TYPE_BOOL:
            return true;
        case HIR_TYPE_POINTER:
            return a->pointer_to == b->pointer_to;
        case HIR_TYPE_GEN:
            return a->gen_param == b->gen_param;
        case HIR_TYPE_INT:
            return
                a->integer.is_signed == b->integer.is_signed && 
                a->integer.size == b->integer.size;
        case HIR_TYPE_FLOAT:
            return a->float_size == b->float_size;
        case HIR_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->function.args); i++) {
                if (a->function.args[i] != b->function.args[i]) {
                    return false;
                }
            }
            return a->function.returns == b->function.returns;
        case HIR_TYPE_STRUCT:
            if (vec_len(a->structure.fields) != vec_len(b->structure.fields)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields); i++) {
                if (a->structure.fields[i].type != b->structure.fields[i].type) {
                    return false;
                }
            }
            return true;
        case HIR_TYPE_ARRAY:
            return a->array.length == b->array.length && a->array.of == b->array.of;
    }
    UNREACHABLE;
}
