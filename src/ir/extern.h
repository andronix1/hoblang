#pragma once

#include "core/slice.h"
#include "ir/api/decl.h"
#include "ir/api/type.h"

typedef enum {
    IR_EXTERN_FUNC,
    IR_EXTERN_VAR
} IrExternKind;

typedef struct {
    IrExternKind kind;
    Slice name;
    IrTypeId type;
} IrExtern;

static inline IrExtern ir_extern_new(IrExternKind kind, Slice name, IrTypeId type) {
    IrExtern ext = {
        .kind = kind,
        .name = name,
        .type = type
    };
    return ext;
}

typedef struct {
    IrExtern ext;
    IrDeclId decl_id;
} IrExternInfo;

static inline IrExternInfo ir_extern_info_new(IrExtern ext, IrDeclId decl_id) {
    IrExternInfo info = {
        .ext = ext,
        .decl_id = decl_id
    };
    return info;
}

