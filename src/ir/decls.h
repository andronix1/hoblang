#pragma once

#include "ir/api/type.h"
#include <stdbool.h>

typedef struct {
    bool filled;
    IrTypeId type;
} IrDecl;

static inline IrDecl ir_decl_new() {
    IrDecl decl = { .filled = false };
    return decl;
}
