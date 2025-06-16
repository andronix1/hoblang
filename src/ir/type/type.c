#include "type.h"
#include "core/vec.h"
#include "ir/api/type.h"

IrTypeId *ir_type_depends_on(Mempool *mempool, IrType *type) {
    IrTypeId *list = vec_new_in(mempool, IrTypeId);
    switch (type->kind) {
        case IR_TYPE_INT:
        case IR_TYPE_FLOAT:
        case IR_TYPE_BOOL:
        case IR_TYPE_VOID:
        case IR_TYPE_POINTER:
            break;
        case IR_TYPE_FUNCTION:
            vec_push(list, type->function.returns);
            vec_extend(list, type->function.args);
            break;
        case IR_TYPE_STRUCT:
            vec_extend(list, type->structure.fields);
            break;
        case IR_TYPE_ARRAY:
            vec_push(list, type->array.of);
            break;
    }
    return list;
}
