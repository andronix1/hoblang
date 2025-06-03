#pragma once

#include "core/mempool.h"
#include "ir/api/stmt/stmt.h"

typedef struct IrCode {
    IrStmt **stmts;
} IrCode;

IrCode *ir_code_new(Mempool *mempool, IrStmt **stmts);
