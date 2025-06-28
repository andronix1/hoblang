#include "type.h"
#include "core/assert.h"
#include "core/vec.h"

bool hir_type_eq(const HirType *a, const HirType *b) {
    if (a->kind != b->kind) {
        return false;
    }
    switch (a->kind) {
        case HIR_TYPE_VOID: case HIR_TYPE_BOOL: return true;
        case HIR_TYPE_POINTER: return hir_type_eq(a->pointer_to, b->pointer_to);
        case HIR_TYPE_GEN: return a->gen_param == b->gen_param;
        case HIR_TYPE_INT: return a->integer.is_signed == b->integer.is_signed && a->integer.size == b->integer.size;
        case HIR_TYPE_FLOAT: return a->float_size == b->float_size;
        case HIR_TYPE_FUNCTION:
            if (vec_len(a->function.args) != vec_len(b->function.args)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->function.args); i++) {
                if (hir_type_eq(a->function.args[i], b->function.args[i])) {
                    return false;
                }
            }
            return a->function.returns == b->function.returns;
        case HIR_TYPE_STRUCT:
            if (vec_len(a->structure.fields) != vec_len(b->structure.fields)) {
                return false;
            }
            for (size_t i = 0; i < vec_len(a->structure.fields); i++) {
                if (hir_type_eq(a->structure.fields[i].type, b->structure.fields[i].type)) {
                    return false;
                }
            }
            return true;
        case HIR_TYPE_ARRAY: return a->array.length == b->array.length && a->array.of == b->array.of;
    }
    UNREACHABLE;
}

HirType *hir_type_new_gen(Mempool *mempool, HirGenParamId gen_param)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_GEN;
        out->gen_param = gen_param;
    )

HirType *hir_type_new_void(Mempool *mempool)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_VOID;
    )

HirType *hir_type_new_pointer(Mempool *mempool, HirType *id)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_POINTER;
        out->pointer_to = id;
    )

HirType *hir_type_new_array(Mempool *mempool, HirType *of, size_t length)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_ARRAY;
        out->array.of = of;
        out->array.length = length;
    )

HirType *hir_type_new_bool(Mempool *mempool)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_BOOL;
    )

HirType *hir_type_new_int(Mempool *mempool, HirTypeIntSize size, bool is_signed)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_INT;
        out->integer.size = size;
        out->integer.is_signed = is_signed;
    )

HirType *hir_type_new_float(Mempool *mempool, HirTypeFloatSize size)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_FLOAT;
        out->float_size = size;
    )

HirType *hir_type_new_struct(Mempool *mempool, HirTypeStructField *fields)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_STRUCT;
        out->structure.fields = fields;
    )

HirType *hir_type_new_function(Mempool *mempool, HirType **args, HirType *returns)
    MEMPOOL_CONSTRUCT(HirType,
        out->kind = HIR_TYPE_FUNCTION;
        out->function.args = args;
        out->function.returns = returns;
    )
