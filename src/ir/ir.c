#include "ir.h"
#include "core/assert.h"

IrTypeId ir_type_record_resolve_simple(const Ir *ir, IrTypeId id) {
    IrTypeInfo *info = &ir->types[id];
    switch (info->kind) {
        case IR_TYPE_INFO_RECORD:
            assert(info->record.filled);
            return ir_type_record_resolve_simple(ir, info->record.id);
        case IR_TYPE_INFO_SIMPLE:
            return id;
    }
    UNREACHABLE;
}
