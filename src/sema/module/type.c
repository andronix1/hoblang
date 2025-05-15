#include "type.h"
#include "core/mempool.h"
#include <string.h>

#define CONSTRUCT(KIND, FIELDS) MEMPOOL_CONSTRUCT(SemaType, { out->kind = KIND; out->in = NULL; FIELDS })

SemaType *sema_type_new_primitive_int(Mempool *mempool, SemaPrimitiveIntSize size, bool is_signed)
    CONSTRUCT(SEMA_TYPE_PRIMITIVE,
        out->primitive.kind = SEMA_PRIMITIVE_INT;
        out->primitive.integer.size = size;
        out->primitive.integer.is_signed = is_signed;
    )

SemaType *sema_type_new_primitive_float(Mempool *mempool, SemaPrimitiveFloat kind)
    CONSTRUCT(SEMA_TYPE_PRIMITIVE,
        out->primitive.kind = SEMA_PRIMITIVE_FLOAT;
        out->primitive.fp = kind;
    )

SemaType *sema_type_new_primitive_void(Mempool *mempool)
    CONSTRUCT(SEMA_TYPE_PRIMITIVE,
        out->primitive.kind = SEMA_PRIMITIVE_VOID;
    )

SemaType *sema_type_new_primitive_bool(Mempool *mempool)
    CONSTRUCT(SEMA_TYPE_PRIMITIVE,
        out->primitive.kind = SEMA_PRIMITIVE_BOOL;
    )

SemaTypeStructField sema_type_struct_field_new(bool is_local, SemaType *type) {
    SemaTypeStructField field = {
        .is_local = is_local,
        .type = type
    };
    return field;
}

SemaType *sema_type_new_struct(Mempool *mempool, SemaTypeStructField *fields_map)
    CONSTRUCT(SEMA_TYPE_STRUCT,
        out->structure.fields_map = fields_map;
    )

SemaType *sema_type_new_function(Mempool *mempool, SemaType **args, SemaType *returns)
    CONSTRUCT(SEMA_TYPE_FUNCTION,
        out->function.args = args;
        out->function.returns = returns;
    )

SemaType *sema_type_new_pointer(Mempool *mempool, SemaType *to)
    CONSTRUCT(SEMA_TYPE_POINTER,
        out->pointer_to = to;
    )

SemaType *sema_type_new_slice(Mempool *mempool, SemaType *of)
    CONSTRUCT(SEMA_TYPE_SLICE,
        out->slice_of = of;
    )

SemaType *sema_type_new_alias(Mempool *mempool, const SemaModule *in, const SemaType *type) {
    SemaType *result = mempool_alloc(mempool, SemaType);
    memcpy(result, type, sizeof(SemaType));
    result->in = in;
    return result;
}
