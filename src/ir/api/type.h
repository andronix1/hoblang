#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef size_t IrTypeId;

typedef struct IrType IrType;

bool ir_type_eq(const IrType *a, const IrType *b);
