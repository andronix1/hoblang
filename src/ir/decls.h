#pragma once

#include "ir/api/type.h"
#include "ir/mut.h"
#include <stdbool.h>

typedef struct {
    IrMutability mutability;
    bool filled;
    IrTypeId type;
} IrDecl;

static inline IrDecl ir_decl_new() {
    IrDecl decl = { .filled = false };
    return decl;
}

static inline void ir_decl_fill(IrDecl *decl, IrMutability mutability, IrTypeId id) {
    decl->filled = true;
    decl->mutability = mutability;
    decl->type = id;
}
