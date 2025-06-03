#pragma once

#include "ir/type/type.h"
#include <stdbool.h>

typedef enum {
    IR_TYPE_INFO_SIMPLE,
    IR_TYPE_INFO_RECORD,
} IrTypeInfoKind;

typedef struct {
    IrTypeInfoKind kind;

    union {
        IrType simple;
        struct {
            bool filled;
            IrTypeId id;
        } record;
    };
} IrTypeInfo;

static inline IrTypeInfo ir_type_info_new_record() {
    IrTypeInfo info = {
        .kind = IR_TYPE_INFO_RECORD,
        .record = {
            .filled = false
        },
    };
    return info;
}

static inline IrTypeInfo ir_type_info_new_simple(IrType type) {
    IrTypeInfo info = {
        .kind = IR_TYPE_INFO_SIMPLE,
        .simple = type,
    };
    return info;
}


