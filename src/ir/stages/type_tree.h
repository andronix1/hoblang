#pragma once

#include "ir/api/ir.h"
#include "ir/api/type.h"
#include <stdbool.h>

typedef struct {
    IrTypeId id;
    IrTypeId what;
} IrTypeCrossReference;

IrTypeCrossReference *ir_type_check_tree(Ir *ir);
