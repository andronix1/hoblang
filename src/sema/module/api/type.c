#include "type.h"
#include "core/vec.h"
#include "sema/module/type.h"

IrTypeId sema_type_ir_id(SemaType *type) {
    if (type->aliases) {
        return (*vec_top(type->aliases))->id;
    }
    return type->ir_id;
}
