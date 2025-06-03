#include "ir.h"
#include "ir/ir.h"
#include "core/mempool.h"
#include "ir/type/type.h"
#include <malloc.h>

Ir *ir_new() {
    Ir *ir = malloc(sizeof(Ir));
    Mempool *mempool = ir->mempool = mempool_new(1024);
    ir->types = vec_new_in(mempool, IrTypeInfo);
    return ir;
}

IrTypeId ir_add_simple_type(Ir *ir, IrType type) {
    for (size_t i = 0; i < vec_len(ir->types); i++) {
        IrTypeInfo *info = &ir->types[i];
        if (info->kind != IR_TYPE_INFO_SIMPLE) {
            continue;
        }
        if (ir_type_eq(&info->simple, &type)) {
            return i;
        }
    }
    vec_push(ir->types, ir_type_info_new_simple(type));
    return vec_len(ir->types) - 1;
}

IrTypeId ir_add_type_record(Ir *ir) {
    vec_push(ir->types, ir_type_info_new_record());
    return vec_len(ir->types) - 1;
}

void ir_set_type_record(Ir *ir, IrTypeId id, IrTypeId type_id) {
    IrTypeInfo *info = &ir->types[id];
    assert(info->kind == IR_TYPE_INFO_RECORD);
    assert(!info->record.filled);
    info->record.filled = true;
    info->record.id = type_id;
}

void ir_free(Ir *ir) {
    mempool_free(ir->mempool);
    free(ir);
}
