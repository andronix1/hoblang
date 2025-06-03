#pragma once

#include "ir/api/type.h"
#include "ir/type/type.h"

typedef struct Ir Ir;

Ir *ir_new();

IrTypeId ir_add_simple_type(Ir *ir, IrType type);
IrTypeId ir_add_type_record(Ir *ir);
void ir_set_type_record(Ir *ir, IrTypeId id, IrTypeId type);

void ir_free(Ir *ir);
